PROJECT(VNVIEW)

ADD_EXECUTABLE(vnview
   main.c view.c pref.c file.c edge.c help.c attr.c Adjust.c FsbGQS.c)
TARGET_LINK_LIBRARIES(vnview via vx)

INSTALL(TARGETS vnview
        RUNTIME DESTINATION ${VIA_INSTALL_BIN_DIR}
        COMPONENT RuntimeLibraries)
