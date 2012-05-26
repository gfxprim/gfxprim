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
%include "GP_Linear.h"
%include "GP_Resize.h"

/* Functions returning new allocated context */
%include "GP_Rotate.h"

%newobject GP_FilterMirrorH_Alloc;
%newobject GP_FilterMirrorV_Alloc;
%newobject GP_FilterRotate90_Alloc;
%newobject GP_FilterRotate180_Alloc;
%newobject GP_FilterRotate270_Alloc;
%newobject GP_FilterSymmetry_Alloc;

%include "GP_Dither.h"

%newobject GP_FilterFloydSteinberg_RGB888_Alloc;
%newobject GP_FilterHilbertPeano_RGB888_Alloc;
