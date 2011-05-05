PROJECT(vmedian3d)

ADD_EXECUTABLE(vmedian3d vmedian3d.c)
TARGET_LINK_LIBRARIES(vmedian3d via)

INSTALL(TARGETS vmedian3d
        RUNTIME DESTINATION ${VIA_INSTALL_BIN_DIR}
        COMPONENT RuntimeLibraries)
