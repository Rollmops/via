PROJECT(VIA)

# address warnings by cmake 2.6
cmake_minimum_required(VERSION 2.4)
if(COMMAND cmake_policy)
  cmake_policy(SET CMP0003 NEW)
  cmake_policy(SET CMP0002 NEW)
endif(COMMAND cmake_policy)

# add path for new modules
SET(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH}
	"${CMAKE_SOURCE_DIR}/cmake/Modules")

# configure install destinations
SET(VIA_INSTALL_BIN_DIR "${CMAKE_INSTALL_PREFIX}/bin")
SET(VIA_INSTALL_LIB_DIR "${CMAKE_INSTALL_PREFIX}/lib")
SET(VIA_INSTALL_INCLUDE_DIR "${CMAKE_INSTALL_PREFIX}/include")
SET(VIA_INSTALL_DOC_DIR "${CMAKE_INSTALL_PREFIX}/share/doc/via-bin")

SET(BUILD_VX off CACHE BOOL "build vxlib")

SET(BUILD_APPS off CACHE BOOL "build apps")

# Set default shared library version
# This library version will be applied to all libraries in the package
# unless it is not explicitely for a certain lib.
SET(VIA_VERSION_MAJOR "2")
SET(VIA_VERSION_MINOR "0")
SET(VIA_VERSION_PATCH "0")


SET(VIA_SHAREDLIB_VERSION
    "${VIA_VERSION_MAJOR}.${VIA_VERSION_MINOR}.${VIA_VERSION_PATCH}")

STRING(REGEX MATCH "^[0-9]+" VIA_SHAREDLIB_SOVERSION ${VIA_SHAREDLIB_VERSION})
SET(VIA_LIBRARY_PROPERTIES VERSION ${VIA_SHAREDLIB_VERSION}
                           SOVERSION ${VIA_SHAREDLIB_SOVERSION}
                           CLEAN_DIRECT_OUTPUT 1)

add_subdirectory(viaio)
add_subdirectory(vialib)

# Find some libs
FIND_PACKAGE(X11 REQUIRED)
IF(NOT X11_Xt_FOUND)
	FIND_PACKAGE(X11_Xt REQUIRED)
ENDIF(NOT X11_Xt_FOUND)

FIND_LIBRARY(LIB_GSL "gsl")
FIND_LIBRARY(LIB_CBLAS "cblas")
FIND_PATH(GSL_INC "gsl/gsl_cblas.h")#ugly - cant use generic blas

INCLUDE_DIRECTORIES(${GSL_INC})

# choose subprojects
if(BUILD_APPS)
  add_subdirectory(base)
  add_subdirectory(pgms)
endif(BUILD_APPS)

if(BUILD_VX)
  FIND_PACKAGE(Motif REQUIRED)
  add_subdirectory(vxlib)
endif(BUILD_VX)




# define global include dir for via headers
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/include ${X11_Xutil_INCLUDE_PATH} ${X11_Xt_INCLUDE_PATH})

INSTALL(PROGRAMS via-help DESTINATION bin)
