PROJECT(vdelsmall)

ADD_EXECUTABLE(vdelsmall vdelsmall.c)
TARGET_LINK_LIBRARIES(vdelsmall via)

INSTALL(TARGETS vdelsmall
        RUNTIME DESTINATION ${VIA_INSTALL_BIN_DIR}
        COMPONENT RuntimeLibraries)
