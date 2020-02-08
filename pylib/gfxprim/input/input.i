%include "../common.i"
%module(package="gfxprim.input") c_input

%{
#include <core/gp_debug.h>
#include "input/gp_input.h"
%}

%extend gp_event {
        ~gp_event() {
                GP_DEBUG(2, "[wrapper] gp_event free()");
                free($self);
        }
        gp_event() {
                GP_DEBUG(2, "[wrapper] gp_event malloc()");
                return malloc(sizeof(gp_event));
        }
};

%include "gp_event.h"
