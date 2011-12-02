PROJECT(VIAIOLIB)

# get source files
FILE(GLOB VIAIO_SRC *.c)

ADD_LIBRARY(viaio SHARED ${VIAIO_SRC})
# Set library version when building shared libs.
SET_TARGET_PROPERTIES(viaio PROPERTIES ${VIA_LIBRARY_PROPERTIES})

ADD_LIBRARY(viaio_static STATIC ${VIAIO_SRC})
#The library target "viaio_static" has a default OUTPUT_NAME of "viaio_static", can be changed with
SET_TARGET_PROPERTIES(viaio_static PROPERTIES ${VIA_LIBRARY_PROPERTIES} OUTPUT_NAME "viaio")
MESSAGE(STATUS ${LIB_GSL})

TARGET_LINK_LIBRARIES(viaio m ${LIB_GSL} ${LIB_CBLAS})

# install libraries
INSTALL(TARGETS viaio viaio_static
    RUNTIME DESTINATION ${VIA_INSTALL_BIN_DIR} COMPONENT RuntimeLibraries
    LIBRARY DESTINATION ${VIA_INSTALL_LIB_DIR} COMPONENT RuntimeLibraries
    ARCHIVE DESTINATION ${VIA_INSTALL_LIB_DIR} COMPONENT Development)

# install header files
FILE(GLOB VIAIO_HDR "${CMAKE_SOURCE_DIR}/include/viaio/*.h")
INSTALL(FILES ${VIAIO_HDR}
    DESTINATION ${VIA_INSTALL_INCLUDE_DIR}/viaio
    COMPONENT Development)

