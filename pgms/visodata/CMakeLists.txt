PROJECT(visodata)

ADD_EXECUTABLE(visodata visodata.c)
TARGET_LINK_LIBRARIES(visodata via)

INSTALL(TARGETS visodata
        RUNTIME DESTINATION ${VIA_INSTALL_BIN_DIR}
        COMPONENT RuntimeLibraries)
