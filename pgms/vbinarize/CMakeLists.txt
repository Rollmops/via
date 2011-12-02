PROJECT(vbinarize)

ADD_EXECUTABLE(vbinarize vbinarize.c)
TARGET_LINK_LIBRARIES(vbinarize via)

INSTALL(TARGETS vbinarize
        RUNTIME DESTINATION ${VIA_INSTALL_BIN_DIR}
        COMPONENT RuntimeLibraries)
