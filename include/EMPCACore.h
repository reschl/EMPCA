#include "BridgeManager.h"
#include "ConfigManager.h"
#include "TAPDeviceManager.h"
#include "Types.h"
#include "VirtlibConnection.h"
#include "DHCPHandler.h"

#include <unordered_set>
#include <thread>

#include <tbb/concurrent_queue.h>

#ifndef _EMPCACORE_H_
#define _EMPCACORE_H_

class TAPDeviceManager;
class FilterManager;
class InterfaceFilter;

class EMPCACore {
  public:
    EMPCACore();
    ~EMPCACore();
    
    bool checkIP(uint32_t ip);

    void addRequest(request rq);

    STATUS start(const char* config_file_name);
    
  private:
    std::unordered_set<uint32_t> available_ip_adresses_;
    std::vector<vm> virtual_machines_;
    tbb::concurrent_bounded_queue<request> requests_;
    BridgeManager* bridge_manager_;
    ConfigManager* config_manager_;
    TAPDeviceManager* tap_device_manager_;
    VirtlibConnection* virtlib_connection_;
    FilterManager* filter_manager_;
    DHCPHandler* dhcp_handler_;
    std::string base_template_;
    std::thread worker_thread_;
    
    bool initializeNewVM(uint32_t ip_adress, TAPDevice* tap_device);
    TAPDevice* initializeTAPDevice();
    InterfaceFilter* initializeInterfaceFilter(const char* listen_device);

    STATUS handleOtherVMs();
    STATUS handleStandardTemplate();
    STATUS handleStartVM();
    STATUS handleDHCP();
    void handleRuntime();

    virDomainPtr getVMByCaller(Caller* caller);

    void handleRequest();

    std::string getXmlFromFile(std::string file_name);

    void addIP(uint32_t ip);
};
#endif // _EMPCACORE_H_
