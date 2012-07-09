%include "../common.i"
%module(package="gfxprim.filters") filters_c

%{
#include "filters/GP_Filters.h"
#include "core/GP_Debug.h"
%}

%import ../core/core.i

%include "GP_Filters.h"

/* Listed in GP_Filters.h: */
%include "GP_Point.h"
%ignore GP_Histogram::hist;
%include "GP_Stats.h"
%include "GP_Linear.h"
%include "GP_Resize.h"

%extend GP_FilterParam {
  ~GP_FilterParam() {
    GP_DEBUG(2, "[wrapper] GP_FilterParamFree");
    GP_FilterParamDestroy($self);
  }
}

%newobject GP_FilterParamCreate;
%include "GP_FilterParam.h"

/* Functions returning new allocated context */
%immutable GP_FilterSymmetryNames;

%newobject GP_FilterMirrorH_Alloc;
%newobject GP_FilterMirrorV_Alloc;
%newobject GP_FilterRotate90_Alloc;
%newobject GP_FilterRotate180_Alloc;
%newobject GP_FilterRotate270_Alloc;
%newobject GP_FilterSymmetry_Alloc;
%include "GP_Rotate.h"

%newobject GP_FilterFloydSteinberg_RGB888_Alloc;
%newobject GP_FilterHilbertPeano_RGB888_Alloc;
%include "GP_Dither.h"

%newobject GP_FilterAdditionAlloc;
%newobject GP_FilterMultiplyAlloc;
%newobject GP_FilterDifferenceAlloc;
%newobject GP_FilterMaxAlloc;
%newobject GP_FilterMinAlloc;
%include "GP_Arithmetic.h"

%newobject GP_FilterConvolutionAlloc;
%newobject GP_FilterConvolutionExAlloc;
%include "GP_Convolution.h"

%newobject GP_FilterBlurAlloc;
%newobject GP_FilterBlurExAlloc;
%include "GP_Blur.h"
