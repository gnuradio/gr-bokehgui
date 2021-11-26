# Copyright 2020 Free Software Foundation, Inc.
#
# This file is part of GNU Radio
#
# SPDX-License-Identifier: GPL-3.0-or-later
#

########################################################################
# Check if there is C++ code at all
########################################################################
if(NOT bokehgui_sources)
    MESSAGE(STATUS "No C++ sources... skipping python bindings")
    return()
endif(NOT bokehgui_sources)

########################################################################
# Check for pygccxml
########################################################################
GR_PYTHON_CHECK_MODULE_RAW(
    "pygccxml"
    "import pygccxml"
    PYGCCXML_FOUND
    )

include(GrPybind)

########################################################################
# Python Bindings
########################################################################

list(APPEND bokehgui_python_files
    time_sink_c_proc_python.cc
    time_sink_f_proc_python.cc
    freq_sink_f_proc_python.cc
    vec_sink_c_proc_python.cc
    vec_sink_f_proc_python.cc
    freq_sink_c_proc_python.cc
    waterfall_sink_f_proc_python.cc
    waterfall_sink_c_proc_python.cc
    base_sink_python.cc
    python_bindings.cc)

GR_PYBIND_MAKE_OOT(bokehgui
   ../..
   gr::bokehgui
   "${bokehgui_python_files}")

install(TARGETS bokehgui_python DESTINATION ${GR_PYTHON_DIR}/bokehgui COMPONENT pythonapi)
