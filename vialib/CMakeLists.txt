PROJECT(VIALIB)

# configure source files
FILE(GLOB VIALIB_SRC *.c)

ADD_LIBRARY(via SHARED ${VIALIB_SRC})
# Set library version when building shared libs.
SET_TARGET_PROPERTIES(via PROPERTIES ${VIA_LIBRARY_PROPERTIES})

ADD_LIBRARY(via_static STATIC ${VIALIB_SRC})
#The library target "via_static" has a default OUTPUT_NAME of "via_static", can be changed with
SET_TARGET_PROPERTIES(via_static PROPERTIES ${VIA_LIBRARY_PROPERTIES} OUTPUT_NAME "via")

TARGET_LINK_LIBRARIES(via viaio ${LIB_GSL} ${LIB_CBLAS})

# install library
INSTALL(TARGETS via via_static
    RUNTIME DESTINATION ${VIA_INSTALL_BIN_DIR} COMPONENT RuntimeLibraries
    LIBRARY DESTINATION ${VIA_INSTALL_LIB_DIR} COMPONENT RuntimeLibraries
    ARCHIVE DESTINATION ${VIA_INSTALL_LIB_DIR} COMPONENT Development)

# install header files
FILE(GLOB VIALIB_HDR "${CMAKE_SOURCE_DIR}/include/*.h")
INSTALL(FILES ${VIALIB_HDR}
    DESTINATION ${VIA_INSTALL_INCLUDE_DIR}/via
    COMPONENT Development)

