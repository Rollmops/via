PROJECT(vconvolve3d)

ADD_EXECUTABLE(vconvolve3d vconvolve3d.c)
TARGET_LINK_LIBRARIES(vconvolve3d via)

INSTALL(TARGETS vconvolve3d
        RUNTIME DESTINATION ${VIA_INSTALL_BIN_DIR}
        COMPONENT RuntimeLibraries)
