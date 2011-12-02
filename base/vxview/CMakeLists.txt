PROJECT(VXVIEW)

ADD_EXECUTABLE(vxview
    main.c view.c pref.c file.c edge.c help.c attr.c Adjust.c FsbGQS.c)
TARGET_LINK_LIBRARIES(vxview via vx)

SET(XAPP_DEFAULTS "$ENV{DESTDIR}/etc/X11/app-defaults" CACHE PATH "Path where to put X11-defaults for vxview")

INSTALL(TARGETS vxview
        RUNTIME DESTINATION ${VIA_INSTALL_BIN_DIR})

INSTALL(FILES Vxview
    DESTINATION ${XAPP_DEFAULTS})
