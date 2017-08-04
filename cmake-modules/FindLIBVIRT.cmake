# FindLIBVIRT.cmake

# LIBVIRT_FOUND - System has LIBVIRT
# LIBVIRT_INCLUDES - The LIBVIRT include directories
# LIBVIRT_LIBRARIES - The libraries needed to use LIBVIRT
# LIBVIRT_DEFINITIONS - Compiler switches required for using LIBVIRT

find_package(PkgConfig)
pkg_check_modules(PC_LIBVIRT QUIET libvirt)
set(LIBVIRT_DEFINITIONS ${PC_LIBVIRT_CFLAGS_OTHER})

find_path(LIBVIRT_INCLUDE_DIR
          NAMES libvirt/libvirt.h libvirt/virterror.h
          HINTS ${PC_LIBVIRT_INCLUDEDIR} ${PC_LIBVIRT_INCLUDE_DIRS}
          PATH_SUFFIXES libvirt)

find_library(LIBVIRT_LIBRARY
             NAMES virt libvirt
             HINTS ${PC_LIBVIRT_LIBDIR} ${PC_LIBVIRT_LIBRARY_DIRS})

set(LIBVIRT_LIBRARIES ${LIBVIRT_LIBRARY})
set(LIBVIRT_INCLUDE_DIRS ${LIBVIRT_INCLUDE_DIR})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(LIBVIRT DEFAULT_MSG
                                  LIBVIRT_LIBRARY LIBVIRT_INCLUDE_DIR)

mark_as_advanced(LIBVIRT_INCLUDE_DIR LIBVIRT_LIBRARY)

