PROJECT(vbinsize)

ADD_EXECUTABLE(vbinsize vbinsize.c)
TARGET_LINK_LIBRARIES(vbinsize via)

INSTALL(TARGETS vbinsize
        RUNTIME DESTINATION ${VIA_INSTALL_BIN_DIR}
        COMPONENT RuntimeLibraries)
