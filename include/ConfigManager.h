#ifndef CONFIGMANAGER_H
#define CONFIGMANAGER_H

#include "Types.h"

#include <jsoncpp/json/json.h>
#include <string>
#include <vector>

class ConfigManager {
  public:
    static const std::string BRIDGE;
    static const std::string BRIDGE_DESTROY;
    static const std::string BRIDGE_NAME;
    static const std::string FILTER;
    static const std::string FILTER_ACTION;
    static const std::string FILTER_ACTION_PARAM;
    static const std::string FILTER_RULE;
    static const std::string MAXRUNTIME;
    static const std::string OTHERVMS;
    static const std::string OTHERVMS_ALWAYS_ON;
    static const std::string OTHERVMS_IP_ADDRESS;
    static const std::string OTHERVMS_PATH;
    static const std::string STANDARDTEMPLATE;
    static const std::string STANDARDTEMPLATE_FORMAT;
    static const std::string STANDARDTEMPLATE_PATH;
    static const std::string STARTVM;
    static const std::string STARTVM_IP_ADDRESS;
    static const std::string STARTVM_PATH;
    static const std::string STARTVM_PRE_SCRIPT;
    static const std::string STARTVM_POST_SCRIPT;
    static const std::string DHCP;
    static const std::string DHCP_HOSTS_DIR;
    static const std::string DHCP_PID_FILE;
    static const std::string DHCP_IP_ADDRESS;

    ConfigManager(const char* config_file_name);
    ~ConfigManager();
    
    bool readConfig();
    char* getConfigFileName();
    void dumpConfig(std::ostream os);

    bool getBridgeDestory();
    unsigned long long int getMaxruntime();
    std::string getBridgeName();
    std::string getStandardTemplate();
    std::string getStandardTemplateFormat();
    std::string getDHCPHostsDir();
    std::string getDHCPPidFile();
    unsigned int getDHCPIPAddress();
    std::vector<filter_rule>* getFilter();
    std::vector<vm_template>* getOtherVMs();
    start_vm* getStartVM();
  private:
    Json::Reader reader_;
    Json::Value values_;

    char* config_file_name_;

    bool validateJSON();
};

#endif //CONFIGMANAGER_H
