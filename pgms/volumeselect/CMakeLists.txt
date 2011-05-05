PROJECT(volumeselect)

ADD_EXECUTABLE(volumeselect volumeselect.c)
TARGET_LINK_LIBRARIES(volumeselect via)

INSTALL(TARGETS volumeselect
        RUNTIME DESTINATION ${VIA_INSTALL_BIN_DIR}
        COMPONENT RuntimeLibraries)
