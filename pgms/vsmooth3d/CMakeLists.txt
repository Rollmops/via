PROJECT(vsmooth3d)

ADD_EXECUTABLE(vsmooth3d vsmooth3d.c)
TARGET_LINK_LIBRARIES(vsmooth3d via)

INSTALL(TARGETS vsmooth3d
        RUNTIME DESTINATION ${VIA_INSTALL_BIN_DIR}
        COMPONENT RuntimeLibraries)
