%include "../common.i"
%module(package="gfxprim.loaders") loaders_c

%{
#include "core/GP_Core.h"
#include "loaders/GP_Loaders.h"
%}

%import ../core/core.i

%include "GP_Loaders.h"

%include "GP_JPG.h"
%include "GP_PBM.h"
%include "GP_PGM.h"
%include "GP_PPM.h"
%include "GP_PNG.h"

