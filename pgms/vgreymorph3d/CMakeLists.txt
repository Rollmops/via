PROJECT(vgreymorph3d)

ADD_EXECUTABLE(vgreymorph3d vgreymorph3d.c)
TARGET_LINK_LIBRARIES(vgreymorph3d via)

INSTALL(TARGETS vgreymorph3d
        RUNTIME DESTINATION ${VIA_INSTALL_BIN_DIR}
        COMPONENT RuntimeLibraries)
