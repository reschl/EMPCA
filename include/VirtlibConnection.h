#ifndef _VIRTLIBCONNECTION_H_
#define _VIRTLIBCONNECTION_H_

#include <guestfs.h>
#include <libvirt/libvirt.h>

#include <random>
#include <string>

class VirtlibConnection {
  public:
    VirtlibConnection();
    ~VirtlibConnection();

    bool init();
    
    virDomainPtr createNewVM(const char* tap_device, const char* mac, const std::string base_vm);
    bool destroyVM(virDomainPtr domain);
    bool suspendVM(virDomainPtr domain);
    bool suspendAllVMs();
    bool resumeVM(virDomainPtr domain);
    bool startVM(virDomainPtr domain);
    bool stopVM(virDomainPtr domain);

    virDomainPtr createVM(const char* xml);
    virDomainPtr getVM(const char* name);
    virDomainPtr getVM(const int id);
    virDomainPtr getVM(const unsigned char* uuid);

    bool addTapToDomain(virDomainPtr* domain, const char* tap, const char* mac);

  private:
    static const std::string _interface_xml;
    static const uint64_t _standard_image_size = 21474836480; // 20 GB
    static const char* _standard_image_format; // qcow2

    virConnectPtr libvirt_Connection_;
    guestfs_h* guestfs_handle_;
    std::default_random_engine* random_generator_;

    virDomainPtr cloneTemplate(const char* template_vm);
    char* createImageWithBackingImage(const std::string backing_image, const char* base_fmt);
    char* createXmlDefinition(const char* nic_xml, const char* name, const char* image, const char* backing_image);

    char* createInterfaceDefinition(const char* dev_name, const char* mac);

    char* generateRandomMac();

    void shutdownAllVMs();
};
#endif // _VIRTLIBCONNECTION_H_
