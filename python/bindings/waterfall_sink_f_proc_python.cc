/*
 * Copyright 2021 Free Software Foundation, Inc.
 *
 * This file is part of GNU Radio
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */


// #include <pybind11/complex.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/numpy.h>
namespace py = pybind11;

#include <bokehgui/waterfall_sink_f_proc.h>
// pydoc.h is automatically generated in the build directory
#include <waterfall_sink_f_proc_pydoc.h>

void bind_waterfall_sink_f_proc(py::module& m)
{

    using waterfall_sink_f_proc    = ::gr::bokehgui::waterfall_sink_f_proc;


    py::class_<waterfall_sink_f_proc, gr::bokehgui::base_sink<float>, gr::sync_block, gr::block, gr::basic_block,
        std::shared_ptr<waterfall_sink_f_proc>>(m, "waterfall_sink_f_proc", D(waterfall_sink_f_proc))

        .def(py::init(&waterfall_sink_f_proc::make),
           py::arg("size"),
           py::arg("wintype"),
           py::arg("fc"),
           py::arg("bw"),
           py::arg("name"),
           D(waterfall_sink_f_proc,make)
        )






        .def("reset",&waterfall_sink_f_proc::reset,
            D(waterfall_sink_f_proc,reset)
        )



        .def("set_size",&waterfall_sink_f_proc::set_size,
            py::arg("arg0"),
            D(waterfall_sink_f_proc,set_size)
        )



        .def("get_center_freq",&waterfall_sink_f_proc::get_center_freq,
            D(waterfall_sink_f_proc,get_center_freq)
        )



        .def("get_bandwidth",&waterfall_sink_f_proc::get_bandwidth,
            D(waterfall_sink_f_proc,get_bandwidth)
        )



        .def("get_time_per_fft",&waterfall_sink_f_proc::get_time_per_fft,
            D(waterfall_sink_f_proc,get_time_per_fft)
        )



        .def("buildwindow",&waterfall_sink_f_proc::buildwindow,
            D(waterfall_sink_f_proc,buildwindow)
        )



        .def("set_time_per_fft",&waterfall_sink_f_proc::set_time_per_fft,
            py::arg("arg0"),
            D(waterfall_sink_f_proc,set_time_per_fft)
        )



        .def("set_fft_window",&waterfall_sink_f_proc::set_fft_window,
            py::arg("win"),
            D(waterfall_sink_f_proc,set_fft_window)
        )



        .def("get_wintype",&waterfall_sink_f_proc::get_wintype,
            D(waterfall_sink_f_proc,get_wintype)
        )



        .def("set_frequency_range",&waterfall_sink_f_proc::set_frequency_range,
            py::arg("arg0"),
            py::arg("arg1"),
            D(waterfall_sink_f_proc,set_frequency_range)
        )


        // .def_buffer([] (waterfall_sink_f_proc &m) -> py::buffer_info {
        //     return py::buffer_info(
        //     m.get_plot_data(),                      /* Pointer to buffer */
        //     sizeof(float),                          /* Size of one scalar */
        //     py::format_descriptor<float>::format(), /* Python struct-style format descriptor */
        //     2,                                      /* Number of dimensions */
        //     { m.get_nconnections(), 2*m.get_buff_size() },                 /* Buffer dimensions */
        //     { sizeof(float) * m.get_nconnections(),             /* Strides (in bytes) for each index */
        //       sizeof(float) }
        //     );
        // });

        ;




}
