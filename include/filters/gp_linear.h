// SPDX-License-Identifier: GPL-2.1-or-later
/*
 * Copyright (C) 2009-2012 Cyril Hrubis <metan@ucw.cz>
 */

/*

   Linear Convolution _raw filters.

 */

#ifndef FILTERS_GP_LINEAR_H
#define FILTERS_GP_LINEAR_H

#include <filters/gp_filter.h>

/*
 * Linear convolution.
 *
 * The kernel is array of kw * kh floats and is indexed as two directional
 * array.
 *
 * The src coordinates and size defines rectangle in the source on which the
 * filter operates.
 *
 * The dst coodinates defines start pixel of in the destination pixmap.
 *
 * To define 3x3 average filter
 *
 * kernel[] = {
 *	1, 1, 1,
 *	1, 1, 1,
 *	1, 1, 1,
 * };
 *
 * kw = kh = 3
 *
 * kern_div = 9
 */
int gp_filter_linear_convolution_raw(const gp_pixmap *src,
                                   gp_coord x_src, gp_coord y_src,
                                   gp_size w_src, gp_size h_src,
                                   gp_pixmap *dst,
                                   gp_coord x_dst, gp_coord y_dst,
                                   float kernel[], uint32_t kw, uint32_t kh,
                                   float kern_div, gp_progress_cb *callback);

/*
 * Special cases for convolution only in horizontal/vertical direction.
 *
 * These are about 10-30% faster than the generic implementation (depending on
 * the kernel size, bigger kernel == more savings).
 *
 * These are two are a base for bilinear filters.
 *
 * Both works also in-place.
 */
int gp_filter_hlinear_convolution_raw(const gp_pixmap *src,
                                    gp_coord x_src, gp_coord y_src,
                                    gp_size w_src, gp_size h_src,
                                    gp_pixmap *dst,
                                    gp_coord x_dst, gp_coord y_dst,
                                    float kernel[], uint32_t kw, float kern_div,
                                    gp_progress_cb *callback);

int gp_filter_vlinear_convolution_raw(const gp_pixmap *src,
                                    gp_coord x_src, gp_coord y_src,
                                    gp_size w_src, gp_size h_src,
                                    gp_pixmap *dst,
                                    gp_coord x_dst, gp_coord y_dst,
                                    float kernel[], uint32_t kh, float kern_div,
                                    gp_progress_cb *callback);

/*
 * Applies both horizontal and vertical convolution and takes care of the
 * correct progress callback (both horizontal and vertical kernels are expected
 * to be similar in size).
 */
int gp_filter_vhlinear_convolution_raw(const gp_pixmap *src,
                                     gp_coord x_src, gp_coord y_src,
                                     gp_size w_src, gp_size h_src,
                                     gp_pixmap *dst,
                                     gp_coord x_dst, gp_coord y_dst,
                                     float hkernel[], uint32_t kw, float hkern_div,
                                     float vkernel[], uint32_t kh, float vkern_div,
                                     gp_progress_cb *callback);

/*
 * Prints a kernel into the stdout.
 */
void gp_filter_kernel_print_raw(float kernel[], int kw, int kh, float kern_div);

typedef struct gp_convolution_params {
	const gp_pixmap *src;
	gp_coord x_src;
	gp_coord y_src;
	gp_size w_src;
	gp_size h_src;

	gp_pixmap *dst;
	gp_coord x_dst;
	gp_coord y_dst;

	float *kernel;
	unsigned int kw;
	unsigned int kh;
	float kern_div;

	gp_progress_cb *callback;
} gp_convolution_params;

static inline int gp_filter_convolution_raw(const struct gp_convolution_params *params)
{
	return gp_filter_linear_convolution_raw(params->src, params->x_src,
	                                      params->y_src, params->w_src,
	                                      params->h_src, params->dst,
	                                      params->x_dst, params->y_dst,
	                                      params->kernel, params->kw,
	                                      params->kh, params->kern_div,
	                                      params->callback);
}

static inline int gp_filter_vconvolution_raw(const struct gp_convolution_params *params)
{

	return gp_filter_vlinear_convolution_raw(params->src, params->x_src,
	                                       params->y_src, params->w_src,
	                                       params->h_src, params->dst,
	                                       params->x_dst, params->y_dst,
	                                       params->kernel, params->kh,
	                                       params->kern_div,
	                                       params->callback);
}

static inline int gp_filter_hconvolution_raw(const struct gp_convolution_params *params)
{
	return gp_filter_hlinear_convolution_raw(params->src, params->x_src,
	                                       params->y_src, params->w_src,
	                                       params->h_src, params->dst,
	                                       params->x_dst, params->y_dst,
	                                       params->kernel, params->kw,
	                                       params->kern_div,
	                                       params->callback);
}

#endif /* FILTERS_GP_LINEAR_H */
