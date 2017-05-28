/* -*- c++ -*- */

#define BOKEHGUI_API

%{
#define SWIG_FILE_WITH_INIT
%}

%module "bokehgui"

%include "gnuradio.i"   // the common stuff

%include "numpy.i"

//load generated python docstrings
%include "bokehgui_swig_doc.i"

%{
#include "bokehgui/time_sink_f_proc.h"
%}


%init %{
if(PyArray_API == NULL)
{
    import_array(); 
}

%}

/* %include "numpy.i"
 * 
 */ 
// %apply (float* INPLACE_ARRAY2, int DIM1, int DIM2) {(float *output_items, int nrows, int size)};

%apply (float** ARGOUTVIEWM_ARRAY2, int* DIM1, int* DIM2) {(float **output_items, int* nrows, int* size)};

%include "bokehgui/time_sink_f_proc.h"

GR_SWIG_BLOCK_MAGIC2(bokehgui, time_sink_f_proc);
