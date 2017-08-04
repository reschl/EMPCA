#include "TAPDeviceManager.h"
#include "TAPDevice.h"

TAPDeviceManager* TAPDeviceManager::instance_ = 0;

TAPDeviceManager* TAPDeviceManager::getInstance() {
  if (instance_ == NULL) {
    instance_ = new TAPDeviceManager();
  }
  
  return instance_;
}

void TAPDeviceManager::addCore(EMPCACore *core)
{
    core_ = core;
}

TAPDeviceManager::TAPDeviceManager() :
    sequential_number_(0)
{

}

TAPDeviceManager::~TAPDeviceManager() {
  for(auto dev : devices_) {
      delete dev;
  }
}

TAPDevice* TAPDeviceManager::createNewTapDevice() {
    TAPDevice* tap = new TAPDevice(sequential_number_);

    sequential_number_ += 2;

    devices_.push_back(tap);

    return tap;
}

void TAPDeviceManager::removeTapDevice(TAPDevice *dev)
{
    auto it = devices_.begin();
    for(; it != devices_.end(); it++) {
        if(*it == dev){
            *it = std::move(devices_.back());
            delete dev;
            devices_.pop_back();
            break;
        }
    }
}

bool TAPDeviceManager::checkIP(uint32_t ip)
{
    return core_->checkIP(ip);
}

void TAPDeviceManager::addRequest(request rq)
{
    core_->addRequest(rq);
}
