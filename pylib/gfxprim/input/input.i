%include "../common.i"
%module(package="gfxprim.input") c_input

%{
#include "core/GP_Debug.h"
#include "input/GP_Input.h"
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

%include "GP_Event.h"
