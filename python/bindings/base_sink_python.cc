/*
 * Copyright 2021 Free Software Foundation, Inc.
 *
 * This file is part of GNU Radio
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */



#include <pybind11/complex.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/numpy.h>

namespace py = pybind11;

#include <bokehgui/base_sink.h>
#include <bokehgui/trigger_mode.h>
// pydoc.h is automatically generated in the build directory
#include <base_sink_pydoc.h>


template <typename T>
void bind_base_sink_template(py::module& m, const char* classname)
{

    using base_sink    = ::gr::bokehgui::base_sink<T>;


    py::class_<base_sink, gr::sync_block, gr::block, gr::basic_block,
        std::shared_ptr<base_sink>>(m, classname, D(base_sink), py::buffer_protocol())


        .def("clear_queue",&base_sink::clear_queue,
            D(base_sink,clear_queue)
        )

        .def("get_size",&base_sink::get_size,
            D(base_sink,get_size)
        )

        .def("get_vlen",&base_sink::get_vlen,
            D(base_sink,get_vlen)
        )
        .def("get_name",&base_sink::get_name,
            D(base_sink,get_name)
        )
        .def("get_nconnections",&base_sink::get_nconnections,
            D(base_sink,get_nconnections)
        )

        .def("get_plot_data", [](base_sink &m){
          int buff_size = m.get_buff_size();
          return py::array_t<float>(
            {m.get_buff_cols(), buff_size}, // shape
            m.get_plot_data() // the data pointer
          );
        });
        ;
}

void bind_base_sink(py::module& m)
{
    bind_base_sink_template<float>(m, "base_sink_f");
    bind_base_sink_template<gr_complex>(m, "base_sink_c");

    // Handle the trigger enums
    py::enum_<gr::bokehgui::trigger_mode>(m, "trigger_mode")
    .value("TRIG_MODE_FREE", gr::bokehgui::trigger_mode::TRIG_MODE_FREE)
    .value("TRIG_MODE_AUTO", gr::bokehgui::trigger_mode::TRIG_MODE_AUTO)
    .value("TRIG_MODE_NORM", gr::bokehgui::trigger_mode::TRIG_MODE_NORM)
    .value("TRIG_MODE_TAG", gr::bokehgui::trigger_mode::TRIG_MODE_TAG)
    .export_values();

    py::enum_<gr::bokehgui::trigger_slope>(m, "trigger_slope")
    .value("TRIG_SLOPE_POS", gr::bokehgui::trigger_slope::TRIG_SLOPE_POS)
    .value("TRIG_SLOPE_NEG", gr::bokehgui::trigger_slope::TRIG_SLOPE_NEG)
    .export_values();
}
