PROJECT(vaniso3d)

ADD_EXECUTABLE(vaniso3d vaniso3d.c)
TARGET_LINK_LIBRARIES(vaniso3d via)

INSTALL(TARGETS vaniso3d
        RUNTIME DESTINATION ${VIA_INSTALL_BIN_DIR}
        COMPONENT RuntimeLibraries)
