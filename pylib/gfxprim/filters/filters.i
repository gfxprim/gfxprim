%include "../common.i"
%module(package="gfxprim.filters") c_filters
%include "carrays.i"

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

FILTER_FUNC(Invert);
FILTER_FUNC(Brightness);
FILTER_FUNC(Contrast);
FILTER_FUNC(BrightnessContrast);
FILTER_FUNC(Posterize);
%include "GP_Point.h"

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

%array_functions(float, float_array);

%extend GP_FilterKernel2D {
        ~GP_FilterKernel2D() {
                free($self);
        }
        GP_FilterKernel2D(unsigned int w, unsigned int h,
                          float *kern, float kern_div) {

                GP_FilterKernel2D *kernel = malloc(sizeof(GP_FilterKernel2D));

                if (!kernel) {
                        GP_DEBUG(1, "Malloc failed :(");
                        return NULL;
                }

                kernel->w = w;
                kernel->h = h;
                kernel->div = kern_div;
                kernel->kernel = kern;

                return kernel;
        }
};

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

FILTER_FUNC(ResizeCubic);
FILTER_FUNC(ResizeCubicInt);
%include "GP_ResizeCubic.h"

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

%array_functions(unsigned int, uint_array);

%extend GP_MedianWeights {
        ~GP_MedianWeights() {
                free($self);
        }
        GP_MedianWeights(unsigned int w, unsigned int h,
                         unsigned int *weights) {

                GP_MedianWeights *ret = malloc(sizeof(GP_MedianWeights));

                if (!ret) {
                        GP_DEBUG(1, "Malloc failed :(");
                        return NULL;
                }

                ret->w = w;
                ret->h = h;
                ret->weights = weights;

                return ret;
        }
};

%include "GP_WeightedMedian.h"

/* Sigma filter */
FILTER_FUNC(FilterSigmaEx);
FILTER_FUNC(FilterSigma);
%include "GP_Sigma.h"

/* Gaussian Noise */
FILTER_FUNC(GaussianNoiseAddEx);
FILTER_FUNC(GaussianNoiseAdd);
%include "GP_GaussianNoise.h"

/* Sepia */
FILTER_FUNC(Sepia);
FILTER_FUNC(SepiaEx);
%include "GP_Sepia.h"
