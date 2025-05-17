%include "../common.i"
%module(package="gfxprim.filters") c_filters
%include "carrays.i"

%{
#include <filters/gp_filters.h>
#include <core/gp_debug.h>
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
%include "gp_point.gen.h"

/* Arithmetic filters */
FILTER_FUNC(addition);
FILTER_FUNC(multiply);
FILTER_FUNC(difference);
FILTER_FUNC(max);
FILTER_FUNC(min);
%include "gp_arithmetic.h"

/* TODO: Stats filters */

/* Rotations filters */
FILTER_FUNC(mirror_h);
FILTER_FUNC(mirror_v);
FILTER_FUNC(rotate_90);
FILTER_FUNC(rotate_180);
FILTER_FUNC(rotate_270);
FILTER_FUNC(symmetry);
%include "gp_rotate.h"

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

%include "gp_convolution.h"

/* Blur */
FILTER_FUNC(gaussian_blur_ex);
FILTER_FUNC(gaussian_blur);
%include "gp_blur.h"

/* Resize filters */
FILTER_FUNC(resize);
%include "gp_resize.h"

FILTER_FUNC(resize_nn);
%include "gp_resize_nn.h"

FILTER_FUNC(resize_linear_int);
FILTER_FUNC(resize_linear_lf_int);
%include "gp_resize_linear.h"

FILTER_FUNC(resize_cubic);
FILTER_FUNC(resize_cubic_int);
%include "gp_resize_cubic.h"

/* Ditherings */
FILTER_FUNC(floyd_steinberg);
FILTER_FUNC(sierra);
FILTER_FUNC(sierra_lite);
FILTER_FUNC(hilbert_peano);
%include "gp_dither.gen.h"

/* Laplace and Laplace Edge Sharpening */
FILTER_FUNC(laplace);
FILTER_FUNC(edge_sharpening);
%include "gp_laplace.h"

/* Median */
FILTER_FUNC(median_ex);
FILTER_FUNC(median);
%include "gp_median.h"

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

%include "gp_weighted_median.h"

/* Sigma filter */
FILTER_FUNC(sigma_ex);
FILTER_FUNC(sigma);
%include "gp_sigma.h"

/* Gaussian Noise */
FILTER_FUNC(gaussian_noise_add_ex);
FILTER_FUNC(gaussian_noise_add);
%include "gp_gaussian_noise.h"

/* Sepia */
FILTER_FUNC(sepia);
FILTER_FUNC(sepia_ex);
%include "gp_sepia.h"
