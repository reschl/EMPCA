#include "TAPDevice.h"
#include "TAPDeviceManager.h"

extern "C" {
#include "libnettools/libnettools.h"
}

#include <fcntl.h>
#include <pthread.h>
#include <stddef.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <linux/if.h>
#include <linux/if_tun.h>
#include <sys/ioctl.h>

#include <iostream>

#define max(a,b) ((a)>(b) ? (a):(b))

const char* TAPDevice::_tap_identifier = "tap";
const char* TAPDevice::_clone_dev = "/dev/net/tun";

TAPDevice::TAPDevice(const unsigned int number):
    number_in_(number),
    number_out_(number+1),
    request_id_(0)
{
    tap_in_name_ = getDeviceName(number_in_);
    tap_out_name_ = getDeviceName(number_out_);
}

TAPDevice::~TAPDevice() {
    if((fcntl(device_in_, F_GETFD) >= 0) || (fcntl(device_out_, F_GETFD) >= 0)) {
        closeTAP();
    }
    delete tap_in_name_;
    delete tap_out_name_;
    delete tap_in_mac_;
}

bool TAPDevice::start() {
    device_in_ = openTAP(tap_in_name_);
    device_out_ = openTAP(tap_out_name_);

    if(!up()) {
        closeTAP();
        return false;
    }

    tap_thread_ = std::thread([=] { TAPDevice::analyzeTraffic(); } );

    if(!tap_thread_.joinable()) {
        stop();
        return false;
    }

    return true;
}

bool TAPDevice::stop() {
    if(pthread_cancel(tap_thread_.native_handle()) != 0) {
        return false;
    }

    tap_thread_.join();
    if(!down()) {
        return false;
    }
    return closeTAP();
}

unsigned int TAPDevice::openTAP(const char* dev) {
    struct ifreq ifr;

    unsigned int device = open(_clone_dev, O_RDWR);

    if(device < 0) {
        return -1;
    }

    memset(&ifr, 0, sizeof(ifr));
    ifr.ifr_flags = (IFF_TAP | IFF_NO_PI);
    strncpy(ifr.ifr_name, dev, IFNAMSIZ);

    if(ioctl(device, TUNSETIFF, (void *) &ifr) < 0) {
        return -1;
    }

    /*int flags = fcntl(device_in_, F_GETFL, 0);
    flags |= O_NONBLOCK;
    if(fcntl(device_, F_SETFL, flags) < 0) {
        return false;
    }*/

    tap_in_mac_ = getHWAddress(tap_in_name_);

    return device;
}

bool TAPDevice::closeTAP() {
    if(!down()) {
        return false;
    }

    if((ioctl(device_in_, TUNSETPERSIST, 0) < 0) || (ioctl(device_out_, TUNSETPERSIST, 0) < 0)) {
        return false;
    }

    close(device_in_);
    close(device_out_);

    device_in_ = -1;
    device_out_ = -1;

    return true;
}

void TAPDevice::analyzeTraffic() {
    fd_set fds;
    char buf[_bufsize];
    size_t l;

    signed int fm = max(device_in_, device_out_) + 1;

    while(1) {
        FD_ZERO(&fds);
        FD_SET(device_in_, &fds);
        FD_SET(device_out_, &fds);

        select(fm, &fds, NULL, NULL, NULL);

        if( FD_ISSET(device_in_, &fds) ) {
            l = read(device_in_,buf,sizeof(buf));
            struct ethhdr * header = (struct ethhdr *) buf;
            if(ntohs(header->h_proto) == ARP_IDENTIFIER) {
                char* arphead = buf + ARP_OFFSET;
                uint32_t ip;
                if(!handleARP((struct arp_header*) arphead, &ip)) {
                    unsigned char* packet = new unsigned char[_bufsize];
                    std::copy(buf, buf + l, packet);
                    createRequest(ip, packet, l);
                    continue;
                }
            }
            write(device_out_,buf,l);
        }
        if( FD_ISSET(device_out_, &fds) ) {
            l = read(device_out_,buf,sizeof(buf));
            // traffic from network is already checked
            write(device_in_,buf,l);
        }
    }
}

char *TAPDevice::getHWAddress(const char *dev) {
    int s;
    struct ifreq ifr;
    s = socket(AF_INET, SOCK_DGRAM, 0);
    strcpy(ifr.ifr_name, dev);
    if(ioctl(s, SIOCGIFHWADDR, &ifr) < 0) {
        return 0;
    }
    //for (i=0; i<HWADDR_len; i++)
    //    sprintf(&MAC_str[i*2],"%02X",((unsigned char*)ifr.ifr_hwaddr.sa_data)[i]);

    char* mac = new char[MAC_STR_LEN];
    size_t j, k;
    for (j=0, k=0; j<6; j++) {
        k+=snprintf(mac+k, MAC_STR_LEN-k, j ? ":%02X" : "%02X", (int)(unsigned int)(unsigned char)ifr.ifr_hwaddr.sa_data[j]);
    }

    close(s);

    return mac;
}

bool TAPDevice::handleARP(arp_header* arp_header, uint32_t* IP)
{
    *IP = ntohl(arp_header->arp_ip_dest);

    return TAPDeviceManager::getInstance()->checkIP(ntohl(arp_header->arp_ip_dest));
}

void TAPDevice::createRequest(uint32_t ip, unsigned char *packet, int packet_length)
{
    request* rq = new request;

    rq->action_ = ACTION::STARTVM;
    rq->ip_ = ip;
    rq->caller_ = this;
    rq->id_ = request_id_;
    request_id_++;

    TAPDeviceManager::getInstance()->addRequest(*rq);

    struct open_request open;
    open.rq_ = rq;
    open.packet_ = packet;
    open.packet_length_ = packet_length;

    open_reuquests_.push_back(open);
}

void TAPDevice::callback(request rq)
{
    for(size_t i = 0; i < open_reuquests_.size(); i++) {
        if(open_reuquests_.at(i).rq_->id_ == rq.id_) {
            write(device_out_, open_reuquests_.at(i).packet_, open_reuquests_.at(i).packet_length_);
            delete open_reuquests_.at(i).packet_;
            delete open_reuquests_.at(i).rq_;
            open_reuquests_.erase(open_reuquests_.begin() + i);
        }
    }
}

char* TAPDevice::getDeviceName(const unsigned int number) {
    std::string device_name = std::string(_tap_identifier);
    device_name += std::to_string(number);
    char* result =  new char[device_name.size()];
    strcpy(result,device_name.c_str());
    return result;
}

bool TAPDevice::up() {
    if(if_init() != 0) {
        return false;
    }
    if(if_promsic(const_cast<char*>(tap_in_name_)) != 0) {
        return false;
    }
    if(if_up(const_cast<char*>(tap_in_name_)) != 0) {
        return false;
    }
    if(if_promsic(const_cast<char*>(tap_out_name_)) != 0) {
        return false;
    }
    if(if_up(const_cast<char*>(tap_out_name_)) != 0) {
        return false;
    }
    return true;
}

bool TAPDevice::down() {
    if(if_init() != 0) {
        return false;
    }
    if(if_down(const_cast<char*>(tap_in_name_)) != 0) {
        return false;
    }
    if(if_down(const_cast<char*>(tap_out_name_)) != 0) {
        return false;
    }
    return true;
}

const char* TAPDevice::getInDevice(){
    return tap_in_name_;
}

const char* TAPDevice::getOutDevice(){
    return tap_out_name_;
}

const char *TAPDevice::getInMac()
{
    return tap_in_mac_;
}
