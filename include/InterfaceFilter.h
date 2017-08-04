#ifndef INTERFACEFILTER_H
#define INTERFACEFILTER_H

#include "Caller.h"
#include "FilterManager.h"

#include <thread>

class FilterManager;

class InterfaceFilter: public Caller
{
    public:
        InterfaceFilter(const uint16_t queue, const char* dev);
        ~InterfaceFilter();

        bool start();
        bool stop();

        // Caller interface
        void callback(request rq);

    private:
        static const std::string IPTABLES_PRE;
        static const std::string IPTABLES_POST;
        static const std::string IPTABLES_REMOVE;
        std::thread filter_thread_;
        bool up_;
        uint16_t queue_;
        char* dev_;
        struct nfq_handle* nh_;
        struct nfq_q_handle* qh_;
        int fd_;
        size_t request_id_;

        struct open_request {
            request* rq_;
            unsigned char* packet_;
            int packet_length_;
        };

        std::vector<open_request> open_reuquests_;

        void checkQueue();
        static int checkPacket(struct nfq_q_handle *qh, struct nfgenmsg *nfmsg, struct nfq_data *nfad, void *data);

        void createRequest(const struct FilterManager::action_answer action_answer, unsigned char* packet, int packet_length);
};

#endif // INTERFACEFILTER_H
