PROJECT(vquickmorph3d)

ADD_EXECUTABLE(vquickmorph3d vquickmorph3d.c)
TARGET_LINK_LIBRARIES(vquickmorph3d via)

INSTALL(TARGETS vquickmorph3d
        RUNTIME DESTINATION ${VIA_INSTALL_BIN_DIR}
        COMPONENT RuntimeLibraries)
