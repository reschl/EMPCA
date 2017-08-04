#
# Use pkg-config to find libguestfs information
#

find_package(PkgConfig)
if ( NOT PKG_CONFIG_FOUND )
  MESSAGE( SEND_ERROR "pkg-config not found")
ENDIF ( NOT PKG_CONFIG_FOUND )
pkg_check_modules(LIBGUESTFS libguestfs)

IF (LIBGUESTFS_FOUND) 
  MESSAGE (STATUS "Found libguestfs libs at ${LIBGUESTFS_LIBRARIES}")
ELSE()
  MESSAGE (SEND_ERROR "libguestfs libs not found!")
ENDIF()

