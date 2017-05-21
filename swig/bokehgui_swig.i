/* -*- c++ -*- */

#define BOKEHGUI_API

%include "gnuradio.i"   // the common stuff

//load generated python docstrings
%include "bokehgui_swig_doc.i"

%{
#include "bokehgui/time_sink_f_proc.h"
%}

%include "bokehgui/time_sink_f_proc.h"

GR_SWIG_BLOCK_MAGIC2(bokehgui, time_sink_f_proc);
