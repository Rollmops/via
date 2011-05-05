PROJECT(vbinmorph3d)

ADD_EXECUTABLE(vbinmorph3d vbinmorph3d.c)
TARGET_LINK_LIBRARIES(vbinmorph3d via)

INSTALL(TARGETS vbinmorph3d
        RUNTIME DESTINATION ${VIA_INSTALL_BIN_DIR}
        COMPONENT RuntimeLibraries)
