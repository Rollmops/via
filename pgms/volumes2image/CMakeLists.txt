PROJECT(volumes2image)

ADD_EXECUTABLE(volumes2image volumes2image.c)
TARGET_LINK_LIBRARIES(volumes2image via)

INSTALL(TARGETS volumes2image
        RUNTIME DESTINATION ${VIA_INSTALL_BIN_DIR}
        COMPONENT RuntimeLibraries)
