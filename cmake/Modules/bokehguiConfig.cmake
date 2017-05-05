INCLUDE(FindPkgConfig)
PKG_CHECK_MODULES(PC_BOKEHGUI bokehgui)

FIND_PATH(
    BOKEHGUI_INCLUDE_DIRS
    NAMES bokehgui/api.h
    HINTS $ENV{BOKEHGUI_DIR}/include
        ${PC_BOKEHGUI_INCLUDEDIR}
    PATHS ${CMAKE_INSTALL_PREFIX}/include
          /usr/local/include
          /usr/include
)

FIND_LIBRARY(
    BOKEHGUI_LIBRARIES
    NAMES gnuradio-bokehgui
    HINTS $ENV{BOKEHGUI_DIR}/lib
        ${PC_BOKEHGUI_LIBDIR}
    PATHS ${CMAKE_INSTALL_PREFIX}/lib
          ${CMAKE_INSTALL_PREFIX}/lib64
          /usr/local/lib
          /usr/local/lib64
          /usr/lib
          /usr/lib64
)

INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(BOKEHGUI DEFAULT_MSG BOKEHGUI_LIBRARIES BOKEHGUI_INCLUDE_DIRS)
MARK_AS_ADVANCED(BOKEHGUI_LIBRARIES BOKEHGUI_INCLUDE_DIRS)

