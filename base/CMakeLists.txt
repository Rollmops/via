PROJECT(VIABASE)

SET(BASEPROGS plaintov pnmtov pngtov rawtov vcatobj vcatbands vcrop vflip
        vimagehisto vinvert vistat vop vrotate vselbands vselect vsynth
        vtopgm vtopnm vnview vconvert)

SET(BUILD_VXVIEW off CACHE BOOL "build vxview")

if(BUILD_VXVIEW)
  SET(BASEPROGS ${BASEPROGS} vxview)
endif(BUILD_VXVIEW)

SUBDIRS(${BASEPROGS})
