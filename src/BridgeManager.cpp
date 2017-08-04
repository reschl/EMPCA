#include "BridgeManager.h"

extern "C" {
#include "libbridge/libbridge.h"
#include "libnettools/libnettools.h"
}

#include <string>
#include <string.h>

#include <iostream>

BridgeManager::BridgeManager(const std::string bridge_name) :
    init_(false)
{
    name_ = new char[bridge_name.length()];
    strcpy(name_, bridge_name.c_str());
}

BridgeManager::BridgeManager(const char* bridge_name) :
    init_(false)
{
    name_ = new char[strlen(bridge_name)];
    strcpy(name_, bridge_name);
}

BridgeManager::~BridgeManager() {
    if(init_) {
        deleteBridge();
    }
    if(name_) {
        delete name_;
    }
}

bool BridgeManager::addTAP(const char* tap_device) {
    if(!init_) {
        return false;
    }
    switch(br_add_interface(name_, tap_device)) {
    case 0:
        return true;
    case ENODEV:
        return false; //if device does not exist it can't be in the bridge
    case EBUSY:
        return true; //already in the bridge
    case ELOOP:
        return false; //can't add a bridge to a bridge
    default:
        return false;
    }

    return false;
}

bool BridgeManager::removeTAP(const char* tap_device) {
    if(!init_) {
        return false;
    }
    switch(br_del_interface(name_, tap_device)) {
    case 0:
        return true;
    case ENODEV:
        return true; //device does not exist -> it can't be in the bridge
    case EINVAL:
        return true; //device was not in the bridge
    default:
        return false;
    }

    return false;
}

bool BridgeManager::init() {
    if(init_) {
        return true;
    }

    if(br_init()) {
        return false;
    }

    if(!createBridge()) {
        return false;
    }

    if(!bridgeUp()) {
        deleteBridge();
        return false;
    }

    init_ = true;
    return true;
}

bool BridgeManager::shutdown() {
    if(!init_) {
        return true;
    }

    if(!bridgeDown()) {
        return false;
    }

    if(deleteBridge()) {
        init_ = false;
        br_shutdown();
        return true;
    }
    return false;
}

char* BridgeManager::getName() {
    return name_;
}

bool BridgeManager::createBridge() {
    int err;
    switch(err = br_add_bridge(name_)) {
    case 0:
        return true;
    case EEXIST:
        return true; //just reuse existing bridges
    default:
        std::cout << "createBridge: " << err << std::endl;
        return false;
    }

    return false;
}

bool BridgeManager::deleteBridge() {
    switch(br_del_bridge(name_)) {
    case 0:
        return true;
    case ENXIO:
        return true; //bridge disappeared?!
    case EBUSY:
        return false;
    default:
        return true;
    }

    return false;
}

bool BridgeManager::bridgeUp() {
    if(if_init() != 0) {
        return false;
    }
    if(if_up(name_) != 0) {
        return false;
    }
    return true;
}

bool BridgeManager::bridgeDown() {
    if(if_init() != 0) {
        return false;
    }
    if(if_down(name_) != 0) {
        return false;
    }
    return true;
}
