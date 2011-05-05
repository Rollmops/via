PROJECT(VSELBANDS)

ADD_EXECUTABLE(vselbands SelSlices.c  vselbands.c)
TARGET_LINK_LIBRARIES(vselbands via)

INSTALL(TARGETS vselbands
        RUNTIME DESTINATION ${VIA_INSTALL_BIN_DIR}
        COMPONENT RuntimeLibraries)
