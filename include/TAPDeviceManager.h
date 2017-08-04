#ifndef _TAPDEVICEMANAGER_H_
#define _TAPDEVICEMANAGER_H_

#include "EMPCACore.h"
#include "TAPDevice.h"
#include "Types.h"


#include <vector>

class TAPDeviceManager {
  public:
    ~TAPDeviceManager();
    static TAPDeviceManager* getInstance();
    void addCore(EMPCACore* core);
  
    TAPDevice* createNewTapDevice();
    void removeTapDevice(TAPDevice* dev);
    
    bool checkIP(uint32_t ip);

    void addRequest(request rq);

  private:
    TAPDeviceManager();
    
    static TAPDeviceManager* instance_;
    EMPCACore* core_;
    std::vector<TAPDevice*> devices_;
    size_t sequential_number_;
};
#endif // _TAPDEVICEMANAGER_H_
