#ifndef _TAPDEVICE_H_
#define _TAPDEVICE_H_

#include "Caller.h"

#include <fstream>
#include <thread>

class TAPDevice: public Caller {
  public:
    TAPDevice(const unsigned int);
    ~TAPDevice();

    bool start();
    bool stop();

    const char* getInDevice();
    const char* getOutDevice();
    const char *getInMac();

    // Caller interface
    void callback(request rq);
      
  private:
    static const char* _tap_identifier;
    static const char* _clone_dev;
    static const unsigned int _bufsize = 1600; //slighty larger than MTU
    static const unsigned int ARP_OFFSET = 0xE;
    static const unsigned short ARP_IDENTIFIER = 0x0806;
    static const size_t MAC_STR_LEN = 18;

    typedef struct arp_header
    {
        uint16_t arp_hard_type;
        uint16_t arp_proto_type;
        uint8_t  arp_hard_size;
        uint8_t  arp_proto_size;
        uint16_t arp_op;
        uint8_t  arp_eth_source[6];
        uint32_t arp_ip_source;
        uint8_t  arp_eth_dest[6];
        uint32_t arp_ip_dest;
    } __attribute__ ((packed)) arp_t;

    // VM - TAP_in - APP - TAP_out - Bridge
    unsigned int number_in_;  // towards VM from network
    char* tap_in_name_;
    char*  tap_in_mac_;
    unsigned int number_out_; // from VM to network
    char* tap_out_name_;
    bool stop_;
    signed int device_in_;
    signed int device_out_;
    std::thread tap_thread_;
    size_t request_id_;

    struct open_request {
        request* rq_;
        unsigned char* packet_;
        int packet_length_;
    };

    std::vector<open_request> open_reuquests_;

    unsigned int openTAP(const char* dev);
    bool closeTAP();
    bool up();
    bool down();
    void analyzeTraffic();

    static char* getDeviceName(const unsigned int number);
    static char* getHWAddress(const char* dev);

    bool handleARP(struct arp_header* arp_header, uint32_t* ip);

    void createRequest(uint32_t ip, unsigned char* packet, int packet_length);


};
#endif //_TAPDEVICE_H_
