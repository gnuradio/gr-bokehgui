# Copyright 2017 Free Software Foundation, Inc.
#
# This file is part of GNU Radio
#
# GNU Radio is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3, or (at your option)
# any later version.
#
# GNU Radio is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with GNU Radio; see the file COPYING.  If not, write to
# the Free Software Foundation, Inc., 51 Franklin Street,
# Boston, MA 02110-1301, USA.

########################################################################
# Setup library
########################################################################
include(GrPlatform) #define LIB_SUFFIX

include_directories(${Boost_INCLUDE_DIR})
link_directories(${Boost_LIBRARY_DIRS})
list(APPEND bokehgui_sources
    time_sink_f_proc_impl.cc
    time_sink_c_proc_impl.cc
    freq_sink_f_proc_impl.cc
    freq_sink_c_proc_impl.cc
    waterfall_sink_f_proc_impl.cc
    waterfall_sink_c_proc_impl.cc)

set(bokehgui_sources "${bokehgui_sources}" PARENT_SCOPE)
if(NOT bokehgui_sources)
	MESSAGE(STATUS "No C++ sources... skipping lib/")
	return()
endif(NOT bokehgui_sources)

add_library(gnuradio-bokehgui SHARED ${bokehgui_sources})
target_link_libraries(gnuradio-bokehgui ${Boost_LIBRARIES} ${GNURADIO_ALL_LIBRARIES})
set_target_properties(gnuradio-bokehgui PROPERTIES DEFINE_SYMBOL "gnuradio_bokehgui_EXPORTS")

if(APPLE)
    set_target_properties(gnuradio-bokehgui PROPERTIES
        INSTALL_NAME_DIR "${CMAKE_INSTALL_PREFIX}/lib"
    )
endif(APPLE)

########################################################################
# Install built library files
########################################################################
include(GrMiscUtils)
GR_LIBRARY_FOO(gnuradio-bokehgui RUNTIME_COMPONENT "bokehgui_runtime" DEVEL_COMPONENT "bokehgui_devel")

########################################################################
# Build and register unit test
########################################################################
include(GrTest)

include_directories(${CPPUNIT_INCLUDE_DIRS})

########################################################################
# Print summary
########################################################################
message(STATUS "Using install prefix: ${CMAKE_INSTALL_PREFIX}")
message(STATUS "Building for version: ${VERSION} / ${LIBVER}")

