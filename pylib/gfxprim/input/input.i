%include "../common.i"
%module(package="gfxprim.input") c_input

%{
#include "core/GP_Debug.h"
#include "input/GP_Input.h"
%}

%extend GP_Event {
        ~GP_Event() {
                GP_DEBUG(2, "[wrapper] GP_Event free()");
                free($self);
        }
        GP_Event() {
                GP_DEBUG(2, "[wrapper] GP_Event malloc()");
                return malloc(sizeof(GP_Event));
        }
};

%include "GP_Event.h"
