find_package(PkgConfig)
if ( NOT PKG_CONFIG_FOUND )
  MESSAGE( SEND_ERROR "pkg-config not found")
ENDIF ( NOT PKG_CONFIG_FOUND )
pkg_search_module(LIBNETFILTERQUEUE "libnetfilter_queue")

IF (LIBNETFILTERQUEUE_FOUND) 
  MESSAGE (STATUS "Found libnetfilter_queue libs at ${LIBNETFILTERQUEUE_LIBRARIES}")
ELSE()
  MESSAGE (SEND_ERROR "libnetfilter_queue libs not found!")
ENDIF()


