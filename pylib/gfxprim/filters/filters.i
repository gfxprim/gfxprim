%include "../common.i"
%module(package="gfxprim.filters") c_filters

%{
#include "filters/GP_Filters.h"
#include "core/GP_Debug.h"
%}

/* Listed in GP_Filters.h: */
%include "GP_Point.h"
%ignore GP_Histogram::hist;
%include "GP_Stats.h"
%include "GP_Linear.h"

/* Resize filters */
ERROR_ON_NONZERO(GP_FilterResize);
%newobject GP_FilterResizeAlloc;
ERROR_ON_NULL(GP_FilterResizeAlloc);
%include "GP_Resize.h"

ERROR_ON_NONZERO(GP_FilterResizeNN);
%newobject GP_FilterResizeNNAlloc;
ERROR_ON_NULL(GP_FilterResizeNNAlloc);
%include "GP_ResizeNN.h"

ERROR_ON_NONZERO(GP_FilterResizeLinearInt);
%newobject GP_FilterResizeLinearIntAlloc;
ERROR_ON_NULL(GP_FilterResizeLinearIntAlloc);
ERROR_ON_NONZERO(GP_FilterResizeLinearLFInt);
%newobject GP_FilterResizeLinearLFIntAlloc;
ERROR_ON_NULL(GP_FilterResizeLinearLFIntAlloc);
%include "GP_ResizeLinear.h"

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

%newobject GP_FilterMedianAlloc;
%newobject GP_FilterMedianExAlloc;
%include "GP_Median.h"

%newobject GP_FilterSigmaAlloc;
%newobject GP_FilterSigmaExAlloc;
%include "GP_Sigma.h"
