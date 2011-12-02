PROJECT(vselbig)

ADD_EXECUTABLE(vselbig vselbig.c)
TARGET_LINK_LIBRARIES(vselbig via)

INSTALL(TARGETS vselbig
        RUNTIME DESTINATION ${VIA_INSTALL_BIN_DIR}
        COMPONENT RuntimeLibraries)
