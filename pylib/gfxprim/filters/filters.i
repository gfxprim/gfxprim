%module(package="gfxprim.filters") filters_c

%{
#include "filters/GP_Filters.h"
%}

#define __attribute__(X)

%import ../core/core.i

%include <stdint.i>

%nodefaultctor;

%include "GP_Filters.h"

/* Listed in GP_Filters.h: */
%include "GP_FilterParam.h"
%include "GP_Point.h"
%include "GP_Arithmetic.h"
%ignore GP_Histogram::hist;
%include "GP_Stats.h"
%include "GP_Rotate.h"
%include "GP_Linear.h"
%include "GP_Resize.h"
/* %include "GP_Dither.h"  -- missing symbols */

/* Functions returning new allocated context */
%newobject GP_FilterMirrorHAlloc;
%newobject GP_FilterMirrorVAlloc;
%newobject GP_FilterRotate90Alloc;
%newobject GP_FilterRotate180Alloc;
%newobject GP_FilterRotate270Alloc;
%newobject GP_FilterSymmetryAlloc;
