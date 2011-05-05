PROJECT(VIMAGEHISTO)

ADD_EXECUTABLE(vimagehisto vimagehisto.c DispHisto.c ImageHistogram.c)
TARGET_LINK_LIBRARIES(vimagehisto via vx)

INSTALL(TARGETS vimagehisto
        RUNTIME DESTINATION ${VIA_INSTALL_BIN_DIR}
        COMPONENT RuntimeLibraries)
