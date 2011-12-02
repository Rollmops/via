PROJECT(vderiche3d)

ADD_EXECUTABLE(vderiche3d vderiche3d.c)
TARGET_LINK_LIBRARIES(vderiche3d via)

INSTALL(TARGETS vderiche3d
        RUNTIME DESTINATION ${VIA_INSTALL_BIN_DIR}
        COMPONENT RuntimeLibraries)
