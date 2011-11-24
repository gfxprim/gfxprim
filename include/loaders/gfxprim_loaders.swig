%module gfxprim_loaders_c

%{
#include "core/GP_Core.h"
#include "loaders/GP_Loaders.h"
%}

#define __attribute__(X) 

%import ../core/gfxprim_core.swig

%include <stdint.i>

%nodefaultctor;

%include "GP_Loaders.h"
%include "GP_JPG.h"
%include "GP_PBM.h"
%include "GP_PGM.h"
%include "GP_PPM.h"
%include "GP_PNG.h"

