#include "VirtlibConnection.h"

#include <string.h>
#include <guestfs.h>
#include <libvirt/libvirt.h>
#include <uuid/uuid.h>

#include <ctime>
#include <functional>
#include <iostream>
#include <iomanip>
#include <random>
#include <sstream>
#include <string>

const std::string VirtlibConnection::_interface_xml = "<interface type='direct'><source dev='DEV' mode='passthrough'/><mac address='MAC'/></interface>";
const char* VirtlibConnection::_standard_image_format = "qcow2";

VirtlibConnection::VirtlibConnection() :
    libvirt_Connection_(0)
{
    std::random_device rd;
    random_generator_ = new std::default_random_engine(rd());

    guestfs_handle_ = guestfs_create();

    if (guestfs_handle_ == NULL) {
        perror ("failed to create libguestfs handle");
        exit (EXIT_FAILURE);
    }
}

VirtlibConnection::~VirtlibConnection() {
    shutdownAllVMs();

    if(libvirt_Connection_) {
        virConnectClose(libvirt_Connection_);
    }
    delete random_generator_;
    guestfs_close(guestfs_handle_);
}

bool VirtlibConnection::init() {
    libvirt_Connection_ = virConnectOpen(0);
    if(libvirt_Connection_) {
        return true;
    }
    return false;
}

virDomainPtr VirtlibConnection::createNewVM(const char* tap_device, const char* mac, const std::string base_vm) {
    char* image;
    char* xml;
    std::string name;
    size_t start, end;
    char* nic;
    virDomainPtr vm;

    nic = createInterfaceDefinition(tap_device, mac);
    std::cout << "adding new vm with dev: " << nic << std::endl;
    image = createImageWithBackingImage(base_vm, _standard_image_format);

    start = base_vm.find_last_of('/');
    end = base_vm.find_last_of('.');
    name = base_vm.substr(start+1, base_vm.size() - end);
    name += std::to_string(time(0));

    xml = createXmlDefinition(nic, name.c_str(), image, base_vm.c_str());

    vm = createVM(xml);

    delete image;
    delete xml;
    delete nic;

    return vm;
}

bool VirtlibConnection::suspendVM(virDomainPtr domain) {
    if(virDomainSuspend(domain) != 0) {
        return false;
    }
    return true;
}

bool VirtlibConnection::suspendAllVMs()
{
    auto count = virConnectNumOfDomains(libvirt_Connection_);
    int* ids = new int[count];
    auto domain_count = virConnectListDomains(libvirt_Connection_, ids, count);
    bool ret;
    for(auto i = 0; i < domain_count; i++) {
        auto vm = virDomainLookupByID(libvirt_Connection_, ids[i]);
        ret = suspendVM(vm);
    }
    delete ids;
    return ret;
}

bool VirtlibConnection::resumeVM(virDomainPtr domain) {
    if(virDomainResume(domain) != 0) {
        return false;
    }
    return true;
}

bool VirtlibConnection::startVM(virDomainPtr domain) {
    if(virDomainCreate(domain) != 0) {
        return false;
    }
    return true;
}

bool VirtlibConnection::stopVM(virDomainPtr domain) {
    if(virDomainDestroy(domain) != 0) {
        return false;
    }
    return true;
}

virDomainPtr VirtlibConnection::createVM(const char *xml) {
    return virDomainDefineXML(libvirt_Connection_, xml);
}

virDomainPtr VirtlibConnection::getVM(const char* name) {
    if(libvirt_Connection_) {
        return virDomainLookupByName(libvirt_Connection_, name);
    }
    return nullptr;
}

virDomainPtr VirtlibConnection::getVM(const int id) {
    if(libvirt_Connection_) {
        return virDomainLookupByID(libvirt_Connection_, id);
    }
    return nullptr;
}

virDomainPtr VirtlibConnection::getVM(const unsigned char* uuid) {
    if(libvirt_Connection_) {
        return virDomainLookupByUUID(libvirt_Connection_, uuid);
    }
    return nullptr;
}

bool VirtlibConnection::addTapToDomain(virDomainPtr* domain, const char *tap, const char* mac) {
    /*auto iface = createInterfaceDefinition(tap, mac);
    const char* xml = virInterfaceGetXMLDesc(iface, 0);*/

    char* xml = createInterfaceDefinition(tap, mac);

    char* old_xml = virDomainGetXMLDesc(*domain, VIR_DOMAIN_XML_INACTIVE);

    if(old_xml == 0) {
        return 0;
    }

    std::string os_xml(old_xml);

    free(old_xml);

    auto pos = os_xml.find("<devices>");
    pos += 9;

    std::string new_xml = os_xml.replace(pos, 0, xml);

    virDomainPtr new_domain = virDomainDefineXML(libvirt_Connection_, new_xml.c_str());

    if(new_domain == 0) {
        return false;
    }

    *domain = new_domain;

    delete xml;

    return true;
}

char* VirtlibConnection::createInterfaceDefinition(const char* dev_name, const char* mac) {
    std::string xml(_interface_xml);
    xml.replace(38, 3, dev_name);
    xml.replace(78, 3 , mac);
    std::cout << xml << std::endl;
    //return virInterfaceDefineXML(libvirt_Connection_, xml.c_str(), 0);

    char* result = new char[xml.size()];
    strcpy(result, xml.c_str());
    return result;
}

char* VirtlibConnection::generateRandomMac() {
    std::ostringstream stringStream;
    std::uniform_int_distribution<int> distribution(0,255);
    auto generate = std::bind(distribution, *random_generator_);
    stringStream << "00:";
    unsigned int tp;

    for (unsigned int i = 0; i < 5; i++){
        tp=generate();
        stringStream << std::setfill ('0') << std::setw(2) << std::hex << tp << (i < 4 ? ":" : "");
    }
    char* result =  new char[stringStream.str().size()];
    strcpy(result,stringStream.str().c_str());
    return result;
}

void VirtlibConnection::shutdownAllVMs()
{
    {
        auto count = virConnectNumOfDomains(libvirt_Connection_);
        int* ids = new int[count];
        auto domain_count = virConnectListDomains(libvirt_Connection_, ids, count);
        int ret;
        for(auto i = 0; i < domain_count; i++) {
            auto vm = virDomainLookupByID(libvirt_Connection_, ids[i]);
            ret = virDomainShutdownFlags(vm, VIR_DOMAIN_SHUTDOWN_ACPI_POWER_BTN);
            if(ret != 0) {
                std::cout << "ERROR: could not shutdown VM (" << ret << ")" << std::endl;
            }
        }
        delete ids;
    }
}

char* VirtlibConnection::createImageWithBackingImage(const std::string backing_image, const char* base_fmt) {
    time_t t = time(0);
    std::ostringstream destination_file;
    size_t file_extension = backing_image.find_last_of('.');
    destination_file << backing_image.substr(0, file_extension) << t << backing_image.substr(file_extension, backing_image.size() - file_extension);

    if(guestfs_disk_create(guestfs_handle_, destination_file.str().c_str(), _standard_image_format, -1, GUESTFS_DISK_CREATE_BACKINGFILE, backing_image.c_str(), GUESTFS_DISK_CREATE_BACKINGFORMAT, base_fmt, -1) == 0) {
        char* result =  new char[destination_file.str().size()];
        strcpy(result,destination_file.str().c_str());
        return result;
    } else {
        return 0;
    }
}

char* VirtlibConnection::createXmlDefinition(const char* nic_xml, const char* name, const char *image, const char* backing_image) {
    std::ostringstream xml_stream;
    uuid_t uuid;
    char* xml;
    char uuid_string[37];

    //char* nic_xml = virInterfaceGetXMLDesc(nic, 0);
    uuid_generate(uuid);

    uuid_unparse(uuid, uuid_string);


    xml_stream << "<domain type='kvm'>";
    xml_stream << "<name>" << name << "</name>";
    xml_stream << "<uuid>" << uuid_string << "</uuid>";
    xml_stream << "<memory unit='KiB'>2097152</memory>";
    xml_stream << "<vcpu placement='static'>2</vcpu>";
    xml_stream << "<resource><partition>/machine</partition></resource>";
    xml_stream << "<os><type arch='x86_64' machine='pc-i440fx-2.1'>hvm</type><boot dev='hd'/></os>";
    xml_stream << "<features><acpi/><apic/><pae/></features>";
    xml_stream << "<cpu mode='host-model'><model fallback='forbid'/><topology sockets='1' cores='2' threads='1'/></cpu>";
    xml_stream << "<clock offset='localtime'><timer name='rtc' tickpolicy='catchup' track='wall'/></clock>";
    xml_stream << "<on_poweroff>destroy</on_poweroff><on_reboot>restart</on_reboot><on_crash>restart</on_crash>";
    xml_stream << "<devices><emulator>/usr/bin/kvm</emulator>";
    xml_stream << "<disk type='file' device='disk'><driver name='qemu' type='qcow2'/>";
    xml_stream << "<source file='" << image << "'/>";
    xml_stream << "<backingStore type='file'><format type='qcow2'/><source file='" << backing_image << "'/></backingStore>";
    xml_stream << "<target dev='hda' bus='ide'/><alias name='ide0-0-0'/><address type='drive' controller='0' bus='0' target='0' unit='0'/></disk>";
    xml_stream << "<disk type='file' device='cdrom'><driver name='qemu' type='raw'/><backingStore/><target dev='hdc' bus='ide'/><readonly/><alias name='ide0-1-0'/><address type='drive' controller='0' bus='1' target='0' unit='0'/></disk>";
    xml_stream << "<controller type='usb' index='0'>\
                  <alias name='usb0'/>\
            <address type='pci' domain='0x0000' bus='0x00' slot='0x01' function='0x2'/>\
            </controller>\
            <controller type='pci' index='0' model='pci-root'>\
            <alias name='pci.0'/>\
            </controller>\
            <controller type='ide' index='0'>\
            <alias name='ide0'/>\
            <address type='pci' domain='0x0000' bus='0x00' slot='0x01' function='0x1'/>\
            </controller>";
    xml_stream << nic_xml;
    xml_stream << "<serial type='pty'>\
                  <source path='/dev/pts/1'/>\
            <target port='0'/>\
            <alias name='serial0'/>\
            </serial>\
            <console type='pty' tty='/dev/pts/1'>\
            <source path='/dev/pts/1'/>\
            <target type='serial' port='0'/>\
            <alias name='serial0'/>\
            </console>\
            <input type='tablet' bus='usb'>\
            <alias name='input0'/>\
            </input>\
            <input type='mouse' bus='ps2'/>\
            <input type='keyboard' bus='ps2'/>\
            <graphics type='vnc' port='5900' autoport='yes' listen='127.0.0.1'>\
            <listen type='address' address='127.0.0.1'/>\
            </graphics>\
            <video>\
            <model type='cirrus' vram='9216' heads='1'/>\
            <alias name='video0'/>\
            <address type='pci' domain='0x0000' bus='0x00' slot='0x02' function='0x0'/>\
            </video>\
            <memballoon model='virtio'>\
            <alias name='balloon0'/>\
            <address type='pci' domain='0x0000' bus='0x00' slot='0x04' function='0x0'/>\
            </memballoon>\
            </devices>\
            </domain>";

    xml = new char[xml_stream.str().size() + 1];
    strcpy(xml,xml_stream.str().c_str());

    return xml;
}
