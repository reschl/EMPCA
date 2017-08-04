#include "DHCPHandler.h"

#include <fstream>
#include <csignal>

#include <arpa/inet.h>
#include <netinet/in.h>

DHCPHandler::DHCPHandler(pid_t pid, std::string dir) :
    dhcp_server_pid_(pid),
    hosts_dir_(dir)
{

}

void DHCPHandler::addHosts(const char *mac, const uint32_t ip)
{
    struct in_addr ip_str;

    ip_str.s_addr = htonl(ip);

    char* dot_ip = inet_ntoa(ip_str);

    std::string file_name = hosts_dir_ + "host" + mac;

    std::fstream host_file(file_name, std::ios_base::out);

    host_file << mac << "," << dot_ip << std::endl;

    host_file.close();

    kill(dhcp_server_pid_, SIGHUP);
}
