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
# Include python install macros
########################################################################
include(GrPython)
if(NOT PYTHONINTERP_FOUND)
    return()
endif()

########################################################################
# Install python sources
########################################################################
GR_PYTHON_INSTALL(
    FILES
    __init__.py
    bokeh_plot_config.py
    time_sink_f.py
    time_sink_c.py
    freq_sink_f.py
    freq_sink_c.py
    textbox.py
    label.py
    slider.py
    range_slider.py
    checkbox.py
    radio_button.py
    bokeh_layout.py
    utils.py
    waterfall_sink_f.py
    waterfall_sink_c.py
    const_sink_c.py
    DESTINATION ${GR_PYTHON_DIR}/bokehgui
)
GR_PYTHON_INSTALL(
    FILES
    plots/__init__.py
    plots/waterfall.py
    plots/bokehgui.py
    DESTINATION ${GR_PYTHON_DIR}/bokehgui/plots
)

install(FILES plots/waterfall.ts
  DESTINATION ${GR_PYTHON_DIR}/bokehgui/plots
  PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)

install(FILES scripts/check-port.sh
  DESTINATION ${GR_PYTHON_DIR}/bokehgui/scripts
  PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)

########################################################################
# Handle the unit tests
########################################################################
include(GrTest)

set(GR_TEST_TARGET_DEPS gnuradio-bokehgui)
set(GR_TEST_PYTHON_DIRS ${CMAKE_BINARY_DIR}/swig)
GR_ADD_TEST(qa_time_sink_f ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/qa_time_sink_f.py)
GR_ADD_TEST(qa_time_sink_c ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/qa_time_sink_c.py)
GR_ADD_TEST(qa_freq_sink_f ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/qa_freq_sink_f.py)
GR_ADD_TEST(qa_freq_sink_c ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/qa_freq_sink_c.py)
GR_ADD_TEST(qa_waterfall_sink_f ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/qa_waterfall_sink_f.py)
GR_ADD_TEST(qa_waterfall_sink_c ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/qa_waterfall_sink_c.py)
