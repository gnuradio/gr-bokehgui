/* -*- c++ -*- */

#define BOKEHGUI_API

%module bokehgui

%include "gnuradio.i"   // the common stuff

%include "numpy.i"

//load generated python docstrings
%include "bokehgui_swig_doc.i"

%{
#include "bokehgui/time_sink_f_proc.h"
%}

/* %include "numpy.i"
 * 
 * 
 * %apply (float* INPLACE_ARRAY2, int DIM1, int DIM2) {(float *output_items, int size, int nconnections)};
*/
%include "bokehgui/time_sink_f_proc.h"

GR_SWIG_BLOCK_MAGIC2(bokehgui, time_sink_f_proc);
