#include <string>

#ifndef _BRIDGEMANAGER_H_
#define _BRIDGEMANAGER_H_
class BridgeManager {
  public:
    BridgeManager(const std::string bridge_name);
    BridgeManager(const char* bridge_name);
    ~BridgeManager();

    bool init();
    bool shutdown();
    
    bool addTAP(const char* tap_device);
    bool removeTAP(const char* tap_device);

    char* getName();
    
  private:
    bool init_;
    char* name_;

    bool createBridge();
    bool deleteBridge();

    bool bridgeUp();
    bool bridgeDown();
};
#endif // _BRIDGEMANAGER_H_
