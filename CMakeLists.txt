cmake_minimum_required(VERSION 2.8)

project(empca)

set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake-modules)

find_package(Threads REQUIRED)
#find_package(PCAP REQUIRED)
find_package(LIBVIRT REQUIRED)
find_package(LIBGUESTFS REQUIRED)
if(${LIBGUESTFS_VERSION} LESS 1.25.31)
  message(SEND_ERROR "libguestfs version (${LIBGUESTFS_VERSION}) too old. 1.25.31 is required")
endif(${LIBGUESTFS_VERSION} LESS 1.25.31)
find_package(UUID REQUIRED)
find_package(LIBJSONCPP REQUIRED)
find_package(LIBNETFILTERQUEUE REQUIRED)
find_package(TBB REQUIRED)

find_package(PkgConfig)
pkg_check_modules(GLIB REQUIRED glib-2.0)
include_directories(${GLIB_INCLUDE_DIRS})

include_directories(
  include/
)

add_library(bridge SHARED
  lib/libbridge/libbridge_devif.c
  lib/libbridge/libbridge_if.c
  lib/libbridge/libbridge_init.c
  lib/libbridge/libbridge_misc.c
)

add_library(nettools SHARED
  lib/libnettools/libnettools.c
  lib/libnettools/af.c
  lib/libnettools/ax25.c
  lib/libnettools/ax25_gr.c
  lib/libnettools/ddp.c
  lib/libnettools/ddp_gr.c
  lib/libnettools/getroute.c
  lib/libnettools/inet.c
  lib/libnettools/inet_gr.c
  lib/libnettools/inet_sr.c
  lib/libnettools/ipx.c
  lib/libnettools/ipx_gr.c
  lib/libnettools/ipx_sr.c
  lib/libnettools/netrom.c
  lib/libnettools/netrom_gr.c
  lib/libnettools/netrom_sr.c
  lib/libnettools/proc.c
  lib/libnettools/setroute.c
  lib/libnettools/sockets.c
  lib/libnettools/unix.c
  lib/libnettools/util.c
  lib/libnettools/x25.c
  lib/libnettools/x25_gr.c
  lib/libnettools/x25_sr.c
)

add_executable(${PROJECT_NAME}
  include/Types.h
  src/BridgeManager.cpp
  src/ConfigManager.cpp
  src/DHCPHandler.cpp
  src/EMPCACore.cpp
  src/FilterManager.cpp
  src/InterfaceFilter.cpp
  src/main.cpp
  src/TAPDevice.cpp
  src/TAPDeviceManager.cpp
  src/VirtlibConnection.cpp
)
target_compile_options(${PROJECT_NAME} PRIVATE -pthread -lpthread -std=c++11 -Wall)
target_link_libraries(${PROJECT_NAME} ${CMAKE_THREAD_LIBS_INIT} ${PCAP_LIBRARIES} ${LIBVIRT_LIBRARIES} ${LIBGUESTFS_LIBRARIES} ${UUID_LIBRARIES} ${LIBJSONCPP_LIBRARIES} ${LIBNETFILTERQUEUE_LIBRARIES} ${TBB_LIBRARIES} bridge nettools)

add_dependencies(${PROJECT_NAME} bridge nettools)
