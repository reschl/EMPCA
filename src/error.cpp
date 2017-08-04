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

    xml = new char[xml_stream.str().size()];
    strcpy(xml,xml_stream.str().c_str());

    return xml;
}