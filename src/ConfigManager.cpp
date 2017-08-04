#include "ConfigManager.h"
#include "Types.h"

#include <string.h>

#include <iostream>
#include <fstream>
#include <jsoncpp/json/json.h>

const std::string ConfigManager::BRIDGE = "Bridge";
const std::string ConfigManager::BRIDGE_DESTROY = "Destroy";
const std::string ConfigManager::BRIDGE_NAME = "Name";
const std::string ConfigManager::FILTER = "Filter";
const std::string ConfigManager::FILTER_ACTION = "Action";
const std::string ConfigManager::FILTER_ACTION_PARAM = "ActionParam";
const std::string ConfigManager::FILTER_RULE = "Rule";
const std::string ConfigManager::MAXRUNTIME = "MaxRuntime";
const std::string ConfigManager::OTHERVMS = "OtherVms";
const std::string ConfigManager::OTHERVMS_ALWAYS_ON = "AlwaysOn";
const std::string ConfigManager::OTHERVMS_IP_ADDRESS = "IPAddress";
const std::string ConfigManager::OTHERVMS_PATH = "Path";
const std::string ConfigManager::STANDARDTEMPLATE = "Standardtemplate";
const std::string ConfigManager::STANDARDTEMPLATE_FORMAT = "Format";
const std::string ConfigManager::STANDARDTEMPLATE_PATH = "Path";
const std::string ConfigManager::STARTVM = "StartVm";
const std::string ConfigManager::STARTVM_IP_ADDRESS = "IPAddress";
const std::string ConfigManager::STARTVM_PATH = "Path";
const std::string ConfigManager::STARTVM_PRE_SCRIPT = "PreScript";
const std::string ConfigManager::STARTVM_POST_SCRIPT = "PostScript";
const std::string ConfigManager::DHCP = "DHCP";
const std::string ConfigManager::DHCP_HOSTS_DIR = "HostsDir";
const std::string ConfigManager::DHCP_PID_FILE = "PidFile";
const std::string ConfigManager::DHCP_IP_ADDRESS = "IPAddress";


ConfigManager::ConfigManager(const char* config_file_name) {
    config_file_name_ = new char[strlen(config_file_name)];
    if(!strcpy(config_file_name_, config_file_name)) {
        delete config_file_name_;
        config_file_name_ = 0;
    }

}

ConfigManager::~ConfigManager() {
    if(config_file_name_) {
        delete config_file_name_;
    }
}

bool ConfigManager::readConfig() {
    std::ifstream input(config_file_name_);

    if(!reader_.parse(input, values_)) {
        std::cout << "Error parsing config file!" << std::endl;
        std::cout << reader_.getFormattedErrorMessages() << std::endl;
        return false;
    }

    //std::cout << values_ << std::endl;

    if(!validateJSON()) {
        std::cout << "Invalid config file!" << std::endl;
        return false;
    }

    return true;
}

char* ConfigManager::getConfigFileName() {
    return config_file_name_;
}

void ConfigManager::dumpConfig(std::ostream os) {
    os << values_ << std::endl;
}

bool ConfigManager::validateJSON() {
    // Check bridge settings
    if(!values_[BRIDGE].isObject()) {
        return false;
    }
    if(!values_[BRIDGE][BRIDGE_DESTROY].isBool()) {
        return false;
    }
    if(!values_[BRIDGE][BRIDGE_NAME].isString()) {
        return false;
    }

    // Check filter settings
    if(values_[FILTER].isArray()) {
        for (Json::ArrayIndex i = 0; i < values_[FILTER].size(); i++){
            if(!(values_[FILTER][i][FILTER_ACTION].isString()
                 && (values_[FILTER][i][FILTER_ACTION_PARAM].isString() || values_[FILTER][(int)i][FILTER_ACTION_PARAM].isNull())
                 && values_[FILTER][i][FILTER_RULE].isUInt())) {
                return false;
            }
        }
    } else if(!values_[FILTER].isNull()) {
        return false;
    }

    // Check max runtime
    if(!(values_[MAXRUNTIME].isNumeric() || values_[MAXRUNTIME].isNull())) {
        return false;
    }

    // Check other VM Settings
    if(values_[OTHERVMS].isArray()) {
        for (Json::ArrayIndex  i = 0; i < values_[OTHERVMS].size(); i++){
            if(!(values_[OTHERVMS][i][OTHERVMS_ALWAYS_ON].isBool()
                 && values_[OTHERVMS][i][OTHERVMS_PATH].isString()
                 && values_[OTHERVMS][i][OTHERVMS_IP_ADDRESS].isUInt())) {
                return false;
            }
        }
    } else if(!values_[OTHERVMS].isNull()) {
        return false;
    }

    // Check standard template settings
    if(!values_[STANDARDTEMPLATE].isObject()) {
        return false;
    }
    if(!values_[STANDARDTEMPLATE][STANDARDTEMPLATE_FORMAT].isString()) {
        return false;
    }
    if(!values_[STANDARDTEMPLATE][STANDARDTEMPLATE_PATH].isString()) {
        return false;
    }

    // Check start VM settings
    if(!values_[STARTVM].isObject()) {
        return false;
    }
    if(!values_[STARTVM][STARTVM_PATH].isString()) {
        return false;
    }
    if(!(values_[STARTVM][STARTVM_PRE_SCRIPT].isString() || values_[STARTVM][STARTVM_PRE_SCRIPT].isNull())) {
        return false;
    }
    if(!(values_[STARTVM][STARTVM_POST_SCRIPT].isString() || values_[STARTVM][STARTVM_POST_SCRIPT].isNull())) {
        return false;
    }
    if(!values_[STARTVM][STARTVM_IP_ADDRESS].isUInt()) {
        return false;
    }

    // Check DHCP settings
    if(!values_[DHCP].isObject()) {
        return false;
    }
    if(!values_[DHCP][DHCP_HOSTS_DIR].isString()) {
        return false;
    }
    if(!values_[DHCP][DHCP_PID_FILE].isString()) {
        return false;
    }
    if(!values_[DHCP][DHCP_IP_ADDRESS].isUInt()) {
        return false;
    }


    return true;
}

bool ConfigManager::getBridgeDestory() {
    return values_[BRIDGE][BRIDGE_DESTROY].asBool();
}

std::string ConfigManager::getBridgeName() {
    return values_[BRIDGE][BRIDGE_NAME].asString();
}

std::string ConfigManager::getStandardTemplate()
{
    return values_[STANDARDTEMPLATE][STANDARDTEMPLATE_PATH].asString();
}

std::string ConfigManager::getStandardTemplateFormat()
{
    return values_[STANDARDTEMPLATE][STANDARDTEMPLATE_FORMAT].asString();
}

std::string ConfigManager::getDHCPHostsDir()
{
    return values_[DHCP][DHCP_HOSTS_DIR].asString();
}

std::string ConfigManager::getDHCPPidFile()
{
    return values_[DHCP][DHCP_PID_FILE].asString();
}

unsigned int ConfigManager::getDHCPIPAddress()
{
    return values_[DHCP][DHCP_IP_ADDRESS].asUInt();
}

// vector has to be freed by caller
std::vector<filter_rule>* ConfigManager::getFilter() {
    std::vector<filter_rule>* filter = new std::vector<filter_rule>();

    if(values_[FILTER].isNull()) {
        return filter;
    }

    for (Json::ArrayIndex i = 0; i < values_[FILTER].size(); i++){
        filter_rule fr;
        fr.action_ = values_[FILTER][i][FILTER_ACTION].asString();
        fr.action_param_ = values_[FILTER][i][FILTER_ACTION_PARAM].asString();
        fr.rule_ = values_[FILTER][i][FILTER_RULE].asUInt();
        filter->push_back(fr);
    }

    return filter;
}

unsigned long long int ConfigManager::getMaxruntime() {
    return values_[MAXRUNTIME].asUInt64();
}

// vector has to be freed by caller
std::vector<vm_template>* ConfigManager::getOtherVMs() {
    std::vector<vm_template>* vms = new std::vector<vm_template>();

    if(values_[OTHERVMS].isNull()) {
        return vms;
    }

    for (Json::ArrayIndex i = 0; i < values_[OTHERVMS].size(); i++){
        vm_template vm;
        vm.path_ = values_[OTHERVMS][i][OTHERVMS_PATH].asString();
        vm.always_on_ = values_[OTHERVMS][i][OTHERVMS_ALWAYS_ON].asBool();
        vm.ip_address_ = values_[OTHERVMS][i][OTHERVMS_IP_ADDRESS].isUInt();
        vms->push_back(vm);
    }

    return vms;
}

// start_vm has to be freed by caller
start_vm* ConfigManager::getStartVM() {
    start_vm* sv = new start_vm();

    sv->path_ = values_[STARTVM][STARTVM_PATH].asString();
    sv->pre_script_ = values_[STARTVM][STARTVM_PRE_SCRIPT].asString();
    sv->post_script_ = values_[STARTVM][STARTVM_POST_SCRIPT].asString();
    sv->ip_address_ = values_[STARTVM][STARTVM_IP_ADDRESS].asUInt();

    return sv;
}
