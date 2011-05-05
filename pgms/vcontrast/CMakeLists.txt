PROJECT(vcontrast)

ADD_EXECUTABLE(vcontrast vcontrast.c)
TARGET_LINK_LIBRARIES(vcontrast via)

INSTALL(TARGETS vcontrast
        RUNTIME DESTINATION ${VIA_INSTALL_BIN_DIR}
        COMPONENT RuntimeLibraries)
