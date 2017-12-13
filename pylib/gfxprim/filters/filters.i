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
%newobject  gp_filter_ ## funcname ## _alloc;
ERROR_ON_NULL(gp_filter_ ## funcname ## _alloc);
ERROR_ON_NONZERO(gp_filter_ ## funcname);
%enddef

FILTER_FUNC(invert);
FILTER_FUNC(brightness);
FILTER_FUNC(contrast);
FILTER_FUNC(brightness_contrast);
FILTER_FUNC(posterize);
%include "GP_Point.h"

/* Arithmetic filters */
FILTER_FUNC(addition);
FILTER_FUNC(multiply);
FILTER_FUNC(difference);
FILTER_FUNC(max);
FILTER_FUNC(min);
%include "GP_Arithmetic.h"

/* TODO: Stats filters */

/* Rotations filters */
FILTER_FUNC(mirror_h);
FILTER_FUNC(mirror_v);
FILTER_FUNC(rotate_90);
FILTER_FUNC(rotate_180);
FILTER_FUNC(rotate_270);
FILTER_FUNC(symmetry);
%immutable gp_filter_symmetry_names;
%include "GP_Rotate.h"

/* Convolutions */
FILTER_FUNC(convolution);
FILTER_FUNC(convolution_ex);

%array_functions(float, float_array);

%extend gp_filter_kernel_2d {
        ~gp_filter_kernel_2d() {
                free($self);
        }
        gp_filter_kernel_2d(unsigned int w, unsigned int h,
                  float *kern, float kern_div) {

                gp_filter_kernel_2d *kernel = malloc(sizeof(gp_filter_kernel_2d));

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
FILTER_FUNC(gaussian_blur_ex);
FILTER_FUNC(gaussian_blur);
%include "GP_Blur.h"

/* Resize filters */
FILTER_FUNC(resize);
%include "GP_Resize.h"

FILTER_FUNC(resize_nn);
%include "GP_ResizeNN.h"

FILTER_FUNC(resize_linear_int);
FILTER_FUNC(resize_linear_lf_int);
%include "GP_ResizeLinear.h"

FILTER_FUNC(resize_cubic);
FILTER_FUNC(resize_cubic_int);
%include "GP_ResizeCubic.h"

/* Ditherings */
FILTER_FUNC(floyd_steinberg);
FILTER_FUNC(hilbert_peano);
%include "GP_Dither.h"

/* Laplace and Laplace Edge Sharpening */
FILTER_FUNC(laplace);
FILTER_FUNC(edge_sharpening);
%include "GP_Laplace.h"

/* Median */
FILTER_FUNC(median_ex);
FILTER_FUNC(median);
%include "GP_Median.h"

/* Weighted Median */
FILTER_FUNC(weighted_median_ex);
FILTER_FUNC(weighted_median);

%array_functions(unsigned int, uint_array);

%extend gp_median_weights {
        ~gp_median_weights() {
                free($self);
        }
        gp_median_weights(unsigned int w, unsigned int h,
                         unsigned int *weights) {

                gp_median_weights *ret = malloc(sizeof(gp_median_weights));

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
FILTER_FUNC(sigma_ex);
FILTER_FUNC(sigma);
%include "GP_Sigma.h"

/* Gaussian Noise */
FILTER_FUNC(gaussian_noise_add_ex);
FILTER_FUNC(gaussian_noise_add);
%include "GP_GaussianNoise.h"

/* Sepia */
FILTER_FUNC(sepia);
FILTER_FUNC(sepia_ex);
%include "GP_Sepia.h"
