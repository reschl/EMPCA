#
# Use pkg-config to find libguestfs information
#

find_package(PkgConfig)
if ( NOT PKG_CONFIG_FOUND )
  MESSAGE( SEND_ERROR "pkg-config not found")
ENDIF ( NOT PKG_CONFIG_FOUND )
pkg_check_modules(LIBJSONCPP jsoncpp)

IF (LIBJSONCPP_FOUND) 
  MESSAGE (STATUS "Found LIBJSONCPP libs at ${LIBJSONCPP_LIBRARIES}")
ELSE()
  MESSAGE (SEND_ERROR "jsoncpp libs not found!")
ENDIF()

