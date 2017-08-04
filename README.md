# Requirements
* libglib2.0
* libguestfs > 1.25.31
* libjsoncpp
* libnetfilter-queue
* libmnl
* libtbb
* libuuid
* libvirt

If you are using apt for packet managing you can run the following to install all build dependencies:

```
sudo apt-get install cmake build-essential libglib2.0-dev libguestfs-dev libjsoncpp-dev libnetfilter-queue-dev libmnl-dev libtbb-dev libvirt-dev uuid-dev
```

# Building the application
cmake is used to keep track dependencies and create the needed build files. Building is therefore as simple as:

```
cmake .
make
```


# Configuration
## kvm-qemu with libvirtd
The application requires kvm-qemu to directly connect to existing network devices. Therefore, the privileges of kvm-qemu have to be adjusted.

Edit your `/etc/libvirt/qemu.conf` to include the following:

```
clear_emulator_capabilities = 0

user = "root"

group = "root"

cgroup_device_acl = [
        "/dev/null", "/dev/full", "/dev/zero",
        "/dev/random", "/dev/urandom",
        "/dev/ptmx", "/dev/kvm", "/dev/kqemu",
	"/dev/rtc", "/dev/hpet", "/dev/net/tun",
	]
```

If you are using selinux you may also need to adjust your  `/etc/selinux/config` to the following:

```
SELINUX=permissive
SELINUXTYPE=targeted
```

These settings decrease the hosts security protection so you probably do not want them on machines providing other productiv services.

## dnsmasq
To create new static DHCP host entries the dhcp-hostsdir option of dnsmasq is used. This options was introduced to dnsmasq in version 2.73. If your packet manager does not provide dnsmasq > 2.73 you will need to build it from sources to take advantage of dynamicaly spawned hosts.

A minimal dnsmasq config file can be found in the example folder.

# Running the application
To run the application it has to be provided with one and only one argument. The argument has to be the path to a valid configuration file. A configuration file including all required and optional parameters can be found in the example folder.

During operation a high amount of privileged  operations have to be executed (e.g. creating TAP interfaces, altering interface status). Therefore it is necessary to run the application with root privileges.

The application can be started with `/path/to/config.json`.

# Provided scripts
Provided scripts will be execupted using system() as given. Therefore, they have to have a valid shebang or the provided line has to include the used interpreter.
