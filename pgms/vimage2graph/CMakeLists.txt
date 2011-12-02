PROJECT(vimage2graph)

ADD_EXECUTABLE(vimage2graph vimage2graph.c)
TARGET_LINK_LIBRARIES(vimage2graph via)

INSTALL(TARGETS vimage2graph
        RUNTIME DESTINATION ${VIA_INSTALL_BIN_DIR}
        COMPONENT RuntimeLibraries)
