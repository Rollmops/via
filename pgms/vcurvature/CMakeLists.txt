PROJECT(vcurvature)

ADD_EXECUTABLE(vcurvature vcurvature.c)
TARGET_LINK_LIBRARIES(vcurvature via)

INSTALL(TARGETS vcurvature
        RUNTIME DESTINATION ${VIA_INSTALL_BIN_DIR}
        COMPONENT RuntimeLibraries)
