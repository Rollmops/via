# - FindX11_Xt.cmake
# 
# Author: Thomas Proeger
#
# Try to find the Xt installation on a Unix platform. This module is for
# backwards compatibility to CMake 2.4 and provides functionality from CMake 2.6.
#
# X11_Xt_FOUND         - True if libxt ist available on this system
# X11_Xt_INLCUDE_PATH  - Include path to use libxt
# X11_Xt_LIB           - link agains this lib to us libxt
#
# CAUTION!!: Use this module only with CMake versions prior to 2.6 if the FindX11.cmake
# module don't provide the variables mentioned above.

IF(NOT UNIX)
  MESSAGE(FATAL_ERROR
    "The package FindX11_Xt only supports unix platforms, at the moment."
    )
ENDIF(NOT UNIX)

# these values were taken from the FindX11.cmake Module that comes with CMake 2.6.
SET(X11_INC_SEARCH_PATH
  /usr/pkg/xorg/include
  /usr/X11R6/include 
  /usr/local/include 
  /usr/include/X11
  /usr/openwin/include 
  /usr/openwin/share/include 
  /opt/graphics/OpenGL/include
  /usr/include
  )

SET(X11_LIB_SEARCH_PATH
  /usr/pkg/xorg/lib
  /usr/X11R6/lib
  /usr/local/lib 
  /usr/openwin/lib 
  /usr/lib 
  )


# find include path
FIND_PATH(X11_Xt_INCLUDE_PATH X11/Intrinsic.h
  ${X11_INC_SEARCH_PATH}
	)

# find library
FIND_LIBRARY(X11_Xt_LIB Xt
  ${X11_LIB_SEARCH_PATH}
	)

# everything is fine?
IF(X11_Xt_LIB AND X11_Xt_INCLUDE_PATH)
  SET(X11_Xt_FOUND TRUE)
ENDIF(X11_Xt_LIB AND X11_Xt_INCLUDE_PATH)
