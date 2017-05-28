/* -*- c++ -*- */

#define BOKEHGUI_API

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

%{
#include "bokehgui/time_sink_f_proc.h"
%}

%{
#if PY_VERSION_HEX >= 0x03020000
# define SWIGPY_SLICE_ARG(obj) ((PyObject*) (obj))
#else
# define SWIGPY_SLICE_ARG(obj) ((PySliceObject*) (obj))
#endif
%}



%template(tagVector) std::vector<gr::tag_t>;
%template(tagDoubleVector) std::vector<std::vector<gr::tag_t> >;


%apply (float** ARGOUTVIEWM_ARRAY2, int* DIM1, int* DIM2) {(float **output_items, int* nrows, int* size)};

%include "bokehgui/time_sink_f_proc.h"


GR_SWIG_BLOCK_MAGIC2(bokehgui, time_sink_f_proc);
