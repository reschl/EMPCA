#include "InterfaceFilter.h"
#include "FilterManager.h"

#include <cassert>
#include <cstring>

#include <arpa/inet.h>
#include <libmnl/libmnl.h>
#include <linux/netfilter.h>
#include <linux/netfilter/nfnetlink_queue.h>
#include <libnetfilter_queue/libnetfilter_queue.h>

const std::string InterfaceFilter::IPTABLES_PRE  = "iptables -A INPUT -m physdev --physdev-in ";
const std::string InterfaceFilter::IPTABLES_POST = " -j NFQUEUE --queue-num ";
const std::string InterfaceFilter::IPTABLES_REMOVE = "iptables -D INPUT -m physdev --physdev-in ";

InterfaceFilter::InterfaceFilter(const uint16_t queue, const char *dev) : Caller(),
    up_(false),
    queue_(queue),
    request_id_(0)
{
    dev_ = new char[strlen(dev)];
    strcpy(dev_, dev);
}

InterfaceFilter::~InterfaceFilter()
{
    if(up_) {
        stop();
    }
    delete dev_;
}

bool InterfaceFilter::start()
{
    nh_ = nfq_open();
    assert(nh_ != 0);

    if(nfq_bind_pf(nh_, AF_INET) < 0) {
        return false;
    }

    qh_ = nfq_create_queue(nh_, queue_, &InterfaceFilter::checkPacket, static_cast<void*>(this));
    assert(qh_ != 0);

    if (nfq_set_mode(qh_, NFQNL_COPY_PACKET, 0xffff) == -1) {
        return false;
    }

    fd_ = nfq_fd(nh_);
    if(fd_ > 0) {
        up_ = true;
    }

    filter_thread_ = std::thread([=] { InterfaceFilter::checkQueue(); } );

    if(!filter_thread_.joinable()) {
        stop();
        return false;
    }

    if(system(std::string(IPTABLES_PRE + dev_ + IPTABLES_POST + std::to_string(queue_)).c_str()) != 0) {
        stop();
        return false;
    }

    return true;

}

bool InterfaceFilter::stop()
{
    up_ = false;
    if(pthread_cancel(filter_thread_.native_handle()) != 0) {
        return false;
    }

    filter_thread_.join();

    nfq_destroy_queue(qh_);
    nfq_unbind_pf(nh_, AF_INET);
    if(nfq_close(nh_) != 0) {
        return false;
    }

    if(system(std::string(IPTABLES_REMOVE + dev_ + IPTABLES_POST + std::to_string(queue_)).c_str()) != 0) {
        stop();
        return false;
    }

    return true;
}

void InterfaceFilter::callback(request rq)
{
    for(size_t i = 0; i < open_reuquests_.size(); i++) {
        if(open_reuquests_.at(i).rq_->id_ == rq.id_) {
            write(fd_, open_reuquests_.at(i).packet_, open_reuquests_.at(i).packet_length_);
            delete open_reuquests_.at(i).packet_;
            delete open_reuquests_.at(i).rq_;
            open_reuquests_.erase(open_reuquests_.begin() + i);
        }
    }
}

void InterfaceFilter::checkQueue()
{
    char buf[4096];
    int res;

    while ((res = recv(fd_, buf, sizeof(buf), 0)) && res >= 0) {
        if(nfq_handle_packet(nh_, buf, res) != 0) {
            break;
        }
    }
}

int InterfaceFilter::checkPacket(nfq_q_handle *qh, nfgenmsg *nfmsg, nfq_data *nfad, void *data)
{
    struct nfqnl_msg_packet_hdr *ph = nfq_get_msg_packet_hdr(nfad);
    uint32_t id;
    if (ph){
        id = ntohl(ph->packet_id);
    }
    uint32_t mark = nfq_get_nfmark(nfad);
    if(mark == 0) {
        return nfq_set_verdict(qh, id, NF_ACCEPT, 0, NULL);
    }

    InterfaceFilter* instance = static_cast<InterfaceFilter*>(data);
    if(!instance->up_) {
        return -1;
    }

    FilterManager::action_answer* aw = FilterManager::getInstance()->handleFilter(mark);

    if(aw->action_ == ACTION::DROP) {
        delete aw;
        return nfq_set_verdict(qh, id, NF_DROP, 0, NULL);
    } else if(aw->action_ == ACTION::UNKOWN || aw->action_ == ACTION::RUNSCRIPT) {
        delete aw;
        return nfq_set_verdict(qh, id, NF_ACCEPT, 0, NULL);
    } else {
        unsigned char* nf_packet;
        int len = nfq_get_payload(nfad,&nf_packet);
        unsigned char* packet = new unsigned char[len];
        std::copy(nf_packet, nf_packet+len, packet);
        instance->createRequest(*aw, packet, len);
        delete aw;
        return nfq_set_verdict(qh, id, NF_DROP, 0, NULL);
    }
}

void InterfaceFilter::createRequest(const FilterManager::action_answer action_answer, unsigned char *packet, int packet_length)
{
    request* rq = new request;

    rq->action_ = action_answer.action_;
    rq->arg_ = action_answer.arg_;
    rq->caller_ = this;
    rq->id_ = request_id_;
    request_id_++;

    struct open_request open;
    open.rq_ = rq;
    open.packet_ = packet;
    open.packet_length_ = packet_length;

    open_reuquests_.push_back(open);

    FilterManager::getInstance()->addRequest(rq);

}
