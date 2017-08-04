#include "EMPCACore.h"
#include "InterfaceFilter.h"
#include "Types.h"

#include <algorithm>
#include <cassert>
#include <chrono>
#include <iostream>
#include <thread>
#include <unordered_set>

#include <sys/stat.h>

EMPCACore::EMPCACore() {
}

EMPCACore::~EMPCACore() {
    if(config_manager_->getBridgeDestory()) {
        bridge_manager_->shutdown();
    }

    if(config_manager_ != 0) {
        delete config_manager_;
    }

    if(virtlib_connection_ != 0) {
        delete virtlib_connection_;
    }

    if(tap_device_manager_ != 0) {
        delete tap_device_manager_;
    }

    if(filter_manager_ != 0) {
        delete filter_manager_;
    }

    if(dhcp_handler_ != 0) {
        delete dhcp_handler_;
    }

}

bool EMPCACore::checkIP(uint32_t ip)
{
    if (available_ip_adresses_.find(ip) != available_ip_adresses_.end()) {
        return true;
    }
    return false;
}

void EMPCACore::addRequest(request rq)
{
    requests_.push(rq);
}

STATUS EMPCACore::start(const char* config_file_name) {
    STATUS st;
    config_manager_ = new ConfigManager(config_file_name);
    tap_device_manager_ = TAPDeviceManager::getInstance();
    virtlib_connection_ = new VirtlibConnection();

    tap_device_manager_->addCore(this);

    filter_manager_ = FilterManager::getInstance();
    filter_manager_->addCore(this);

    if(!config_manager_->readConfig()) {
        return STATUS::CONFIG_ERROR;
    }

    st = handleDHCP();
    if(st != STATUS::OK) {
        return st;
    }

    bridge_manager_ = new BridgeManager(config_manager_->getBridgeName());

    assert((config_manager_ != 0) && (tap_device_manager_ != 0) && (virtlib_connection_ != 0) && (bridge_manager_ != 0));

    if(!bridge_manager_->init()) {
        return STATUS::BRIDGE_ERROR;
    }

    if(!virtlib_connection_->init()) {
        return STATUS::VIRTLIB_ERROR;
    }

    st = handleStandardTemplate();
    if(st != STATUS::OK) {
        return st;
    }

    worker_thread_ = std::thread([=] { EMPCACore::handleRequest(); } );

    st = handleOtherVMs();
    if(st != STATUS::OK) {
        return st;
    }

    st = handleStartVM();
    if(st != STATUS::OK) {
        return st;
    }

    handleRuntime();

    return STATUS::OK;
}

bool EMPCACore::initializeNewVM(uint32_t ip_adress, TAPDevice* tap_device) {
    vm new_vm;
    const char* mac = tap_device->getInMac();

    new_vm.domain_ =  virtlib_connection_->createNewVM(tap_device->getInDevice(), mac, base_template_);
    new_vm.tap_ = tap_device;
    new_vm.filter_ = initializeInterfaceFilter(tap_device->getInDevice());

    dhcp_handler_->addHosts(mac, ip_adress);

    new_vm.running_ = virtlib_connection_->startVM(new_vm.domain_);

    addIP(ip_adress);
    virtual_machines_.push_back(new_vm);

    return new_vm.running_;
}

TAPDevice* EMPCACore::initializeTAPDevice() {
    TAPDevice* dev = tap_device_manager_->createNewTapDevice();

    assert(dev != 0);

    dev->start();

    assert(bridge_manager_->addTAP(dev->getOutDevice()));

    return dev;
}

InterfaceFilter *EMPCACore::initializeInterfaceFilter(const char* listen_device)
{
    InterfaceFilter* inter = filter_manager_->createNewInterfaceFilter(listen_device);

    assert(inter != 0);

    inter->start();

    return inter;
}

STATUS EMPCACore::handleOtherVMs() {
    auto vms = config_manager_->getOtherVMs();

    for(auto machine : *vms) {
        vm vm_st;

        vm_st.domain_ =  virtlib_connection_->createVM(getXmlFromFile(machine.path_).c_str());
        vm_st.tap_ = initializeTAPDevice();
        vm_st.filter_ = initializeInterfaceFilter(vm_st.tap_->getInDevice());

        assert((vm_st.domain_ != 0) && (vm_st.tap_ !=0));

        if(!virtlib_connection_->addTapToDomain(&vm_st.domain_, vm_st.tap_->getInDevice(), vm_st.tap_->getInMac())) {
            return STATUS::VIRTLIB_ERROR;
        }

        if(machine.always_on_) {
            if(!virtlib_connection_->startVM(vm_st.domain_)) {
                return STATUS::VIRTLIB_ERROR;
            }
            vm_st.running_ = true;
        } else {
            vm_st.running_ = false;
        }
        addIP(machine.ip_address_);

        virtual_machines_.push_back(vm_st);
    }

    return STATUS::OK;
}

STATUS EMPCACore::handleStandardTemplate() {
    base_template_ = config_manager_->getStandardTemplate();

    if(std::find(std::begin(SUPPORTED_IMAGE_FORMATS), std::end(SUPPORTED_IMAGE_FORMATS), config_manager_->getStandardTemplateFormat()) == SUPPORTED_IMAGE_FORMATS.end()) {
        return STATUS::CONFIG_ERROR;
    }

    return STATUS::OK;
}

STATUS EMPCACore::handleStartVM() {
    vm vm_st;
    start_vm* stvm = config_manager_->getStartVM();
    if(!stvm->pre_script_.empty()) {
        if( system(stvm->pre_script_.c_str()) != 0) {
            return STATUS::START_SCRIPT_ERROR;
        }
    }

    vm_st.tap_ = initializeTAPDevice();
    vm_st.domain_ = virtlib_connection_->createVM(getXmlFromFile(stvm->path_).c_str());
    vm_st.filter_ = initializeInterfaceFilter(vm_st.tap_->getInDevice());

    assert((vm_st.domain_ != 0) && (vm_st.tap_ !=0));

    if(!virtlib_connection_->addTapToDomain(&vm_st.domain_, vm_st.tap_->getInDevice(), vm_st.tap_->getInMac())) {
        return STATUS::VIRTLIB_ERROR;
    }

    if(!virtlib_connection_->startVM(vm_st.domain_)) {
        return STATUS::VIRTLIB_ERROR;
    }

    vm_st.running_ = true;
    virtual_machines_.push_back(vm_st);

    if(!stvm->post_script_.empty()) {
        if( system(stvm->post_script_.c_str()) != 0) {
            return STATUS::START_SCRIPT_ERROR;
        }
    }

    addIP(stvm->ip_address_);

    return STATUS::OK;
}

STATUS EMPCACore::handleDHCP()
{
    auto path = config_manager_->getDHCPHostsDir();
    auto pid_file = config_manager_->getDHCPPidFile();

    pid_t pid;

    std::fstream pid_fs(pid_file, std::ios_base::in);

    if(pid_fs.fail()) {
        return STATUS::CONFIG_ERROR;
    }

    pid_fs >> pid;

    if(path.back() != '/') {
        path += "/";
    }

    struct stat s;
    if( stat(path.c_str(),&s) == 0 )
    {
        if( !(s.st_mode & S_IFDIR))
        {
            return STATUS::CONFIG_ERROR;
        }
    } else {
        return STATUS::CONFIG_ERROR;
    }

    dhcp_handler_ = new DHCPHandler(pid, path);

    addIP(config_manager_->getDHCPIPAddress());

    return STATUS::OK;
}

std::string EMPCACore::getXmlFromFile(std::string file_name) {
    std::ifstream file(file_name);
    std::string line, text;

    while(std::getline(file, line)) {
        text += line + "\n";
    }

    return text;
}

void EMPCACore::addIP(uint32_t ip)
{
    available_ip_adresses_.insert(ip);
}

void EMPCACore::handleRuntime() {
    auto runtime = config_manager_->getMaxruntime();

    if(runtime > 0) {
        std::this_thread::sleep_for(std::chrono::seconds(runtime));
    } else {
        std::cout << "Press enter to exit" << std::endl;
        std::cin.ignore();
    }
}

virDomainPtr EMPCACore::getVMByCaller(Caller *caller)
{
    for(auto machine : virtual_machines_) {
        Caller* tap = machine.tap_;
        Caller* filter = machine.filter_;
        if(tap == caller || filter == caller) {
            return machine.domain_;
        }
    }
    return 0;
}

void EMPCACore::handleRequest()
{
    while(true) {
        request rq;

        requests_.pop(rq);

        switch(rq.action_) {
        case ACTION::FREEZEALL:
            virtlib_connection_->suspendAllVMs();
            break;
        case ACTION::FREEZESRC: {
            auto vm = getVMByCaller(rq.caller_);
            virtlib_connection_->suspendVM(vm);
            break;
        }
        case ACTION::RUNSCRIPT:
            system(rq.arg_.c_str());
            break;
        case ACTION::STARTVM:
            if(rq.ip_ != 0) {
                TAPDevice* tap = initializeTAPDevice();
                if(!initializeNewVM(rq.ip_, tap)) {
                    std::cout << "ERROR: could not start machine for IP: " << std::to_string(rq.ip_) << std::endl;
                }
            } else if(rq.arg_ != "") {
                auto vm = virtlib_connection_->getVM(rq.arg_.c_str());
                virtlib_connection_->startVM(vm);
            }
            break;
        default:
            std::cout << "ERROR: unkown ACTION" << std::endl;
        }
        rq.caller_->callback(rq);
    }
}

