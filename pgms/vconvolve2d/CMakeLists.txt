PROJECT(vconvolve2d)

ADD_EXECUTABLE(vconvolve2d vconvolve2d.c)
TARGET_LINK_LIBRARIES(vconvolve2d via)

INSTALL(TARGETS vconvolve2d
        RUNTIME DESTINATION ${VIA_INSTALL_BIN_DIR}
        COMPONENT RuntimeLibraries)
