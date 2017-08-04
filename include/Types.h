#ifndef TYPES_H
#define TYPES_H

#include <libvirt/libvirt.h>

#include <string>
#include <vector>

class Caller;
class InterfaceFilter;
class TAPDevice;

enum class ACTION {RUNSCRIPT, STARTVM, FREEZESRC, FREEZEALL, DROP, UNKOWN};
enum class HANDLEPACKET {FORWARD, DROP, HOLD};

enum class STATUS {OK, CONFIG_ERROR, BRIDGE_ERROR, START_SCRIPT_ERROR, VIRTLIB_ERROR};


static const std::vector<std::string> SUPPORTED_IMAGE_FORMATS{"qcow2"};

struct ST_VM {
    virDomainPtr domain_;
    TAPDevice* tap_;
    InterfaceFilter* filter_;
    bool running_;
} typedef vm;

struct ST_VM_TEMPLATE {
    std::string path_;
    bool always_on_;
    uint32_t ip_address_;
} typedef vm_template;

struct ST_START_VM {
    std::string path_;
    std::string pre_script_;
    std::string post_script_;
    uint32_t ip_address_;
} typedef start_vm;

struct ST_FILTER_RULE {
    uint32_t rule_;
    std::string action_;
    std::string action_param_;
} typedef filter_rule;

struct ST_REQUEST {
    size_t id_;
    ACTION action_;
    uint32_t ip_;
    std::string arg_;
    Caller* caller_;
} typedef request;

#endif // TYPES_H
