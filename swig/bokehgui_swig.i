/* -*- c++ -*- */

#define BOKEHGUI_API
#define FILTER_API

%{
#define SWIG_FILE_WITH_INIT
%}

%module "bokehgui"

%include "gnuradio.i"   // the common stuff
%include "numpy.i"
%include "std_vector.i"

%init %{
if(PyArray_API == NULL)
{
    import_array(); 
}
%}

//load generated python docstrings
%include "bokehgui_swig_doc.i"

//load trigger modes
%include "bokehgui/trigger_mode.h"

// load the firdes window types
%import "gnuradio/filter/firdes.h"

%{
#include "bokehgui/base_sink.h"
#include "bokehgui/time_sink_f_proc.h"
#include "bokehgui/time_sink_c_proc.h"
#include "bokehgui/freq_sink_f_proc.h"
#include "bokehgui/freq_sink_c_proc.h"
%}

%{
#if PY_VERSION_HEX >= 0x03020000
# define SWIGPY_SLICE_ARG(obj) ((PyObject*) (obj))
#else
# define SWIGPY_SLICE_ARG(obj) ((PySliceObject*) (obj))
#endif
%}

%include "bokehgui/base_sink.h"
%template(baseSink_ff) gr::bokehgui::base_sink<float, float>;
%template(baseSink_cc) gr::bokehgui::base_sink<gr_complex, gr_complex>;

%template(tagVector) std::vector<gr::tag_t>;
%template(tagDoubleVector) std::vector<std::vector<gr::tag_t> >;

%numpy_typemaps(gr_complex, NPY_CFLOAT , int);
%apply (float** ARGOUTVIEW_ARRAY2, int* DIM1, int* DIM2) {(float** output_items, int* nrows, int* size)};
%apply (gr_complex** ARGOUTVIEW_ARRAY2, int* DIM1, int* DIM2) {(gr_complex** output_items, int* nrows, int* size)};

%include "bokehgui/time_sink_f_proc.h"
GR_SWIG_BLOCK_MAGIC2(bokehgui, time_sink_f_proc);
%include "bokehgui/time_sink_c_proc.h"
GR_SWIG_BLOCK_MAGIC2(bokehgui, time_sink_c_proc);
%include "bokehgui/freq_sink_f_proc.h"
GR_SWIG_BLOCK_MAGIC2(bokehgui, freq_sink_f_proc);
%include "bokehgui/freq_sink_c_proc.h"
GR_SWIG_BLOCK_MAGIC2(bokehgui, freq_sink_c_proc);
