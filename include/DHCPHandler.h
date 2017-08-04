#ifndef DHCPHANDLER_H
#define DHCPHANDLER_H

#include <string>

class DHCPHandler
{
    public:
        DHCPHandler(pid_t pid, std::string dir);

        void addHosts(const char* mac, const uint32_t ip);

    private:
        pid_t dhcp_server_pid_;
        std::string hosts_dir_;
};

#endif // DHCPHANDLER_H
