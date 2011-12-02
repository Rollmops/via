PROJECT(vkernel2d)

ADD_EXECUTABLE(vkernel2d vkernel2d.c)
TARGET_LINK_LIBRARIES(vkernel2d via)

INSTALL(TARGETS vkernel2d
        RUNTIME DESTINATION ${VIA_INSTALL_BIN_DIR}
        COMPONENT RuntimeLibraries)
