PROJECT(vtopoclass)

ADD_EXECUTABLE(vtopoclass vtopoclass.c Topoclass.c Junction.c NumComp.c)
TARGET_LINK_LIBRARIES(vtopoclass via)

INSTALL(TARGETS vtopoclass
        RUNTIME DESTINATION ${VIA_INSTALL_BIN_DIR}
        COMPONENT RuntimeLibraries)
