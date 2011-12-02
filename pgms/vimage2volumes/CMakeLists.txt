PROJECT(vimage2volumes)

ADD_EXECUTABLE(vimage2volumes vimage2volumes.c)
TARGET_LINK_LIBRARIES(vimage2volumes via)

INSTALL(TARGETS vimage2volumes
        RUNTIME DESTINATION ${VIA_INSTALL_BIN_DIR}
        COMPONENT RuntimeLibraries)
