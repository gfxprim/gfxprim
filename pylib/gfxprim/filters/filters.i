%include "../common.i"
%module(package="gfxprim.filters") c_filters

%{
#include "filters/GP_Filters.h"
#include "core/GP_Debug.h"
%}

%import ../core/core.i

/*
 * Creates allocating and non-allocating filter definitions
 */
%define FILTER_FUNC(funcname)
%newobject GP_Filter ## funcname ## Alloc;
ERROR_ON_NULL(GP_Filter ## funcname ## Alloc);
ERROR_ON_NONZERO(GP_Filter ## funcname);
%enddef

/* Listed in GP_Filters.h: */
%extend GP_FilterParam {
  ~GP_FilterParam() {
    GP_DEBUG(2, "[wrapper] GP_FilterParamFree");
    GP_FilterParamDestroy($self);
  }
}

%newobject GP_FilterParamCreate;
%include "GP_FilterParam.h"

/* TODO: Point filters, once fixed */

/* Arithmetic filters */
FILTER_FUNC(Addition);
FILTER_FUNC(Multiply);
FILTER_FUNC(Difference);
FILTER_FUNC(Max);
FILTER_FUNC(Min);
%include "GP_Arithmetic.h"

/* TODO: Stats filters */

/* Rotations filters */
FILTER_FUNC(MirrorH);
FILTER_FUNC(MirrorV);
FILTER_FUNC(Rotate90);
FILTER_FUNC(Rotate180);
FILTER_FUNC(Rotate270);
FILTER_FUNC(Symmetry);
%immutable GP_FilterSymmetryNames;
%include "GP_Rotate.h"

/* Convolutions */
FILTER_FUNC(Convolution);
FILTER_FUNC(ConvolutionEx);
%include "GP_Convolution.h"

/* Blur */
FILTER_FUNC(GaussianBlurEx);
FILTER_FUNC(GaussianBlur);
%include "GP_Blur.h"

/* Resize filters */
FILTER_FUNC(Resize);
%include "GP_Resize.h"

FILTER_FUNC(ResizeNN);
%include "GP_ResizeNN.h"

FILTER_FUNC(ResizeLinearInt);
FILTER_FUNC(ResizeLinearLFInt);
%include "GP_ResizeLinear.h"

/* Ditherings */
FILTER_FUNC(FloydSteinberg);
FILTER_FUNC(GP_FilterHilbertPeano);
%include "GP_Dither.h"

/* Laplace and Laplace Edge Sharpening */
FILTER_FUNC(Laplace);
FILTER_FUNC(EdgeSharpening);
%include "GP_Laplace.h"

/* Median */
FILTER_FUNC(MedianEx);
FILTER_FUNC(Median);
%include "GP_Median.h"

/* Weighted Median */
FILTER_FUNC(WeightedMedianEx);
FILTER_FUNC(WeightedMedian);
%include "GP_WeightedMedian.h"

/* Sigma filter */
FILTER_FUNC(GP_FilterSigmaEx);
FILTER_FUNC(GP_FilterSigma);
%include "GP_Sigma.h"

/* Gaussian Noise */
FILTER_FUNC(GaussianNoiseAddEx);
FILTER_FUNC(GaussianNoiseAdd);
%include "GP_GaussianNoise.h"
