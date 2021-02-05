// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2009-2012 Cyril Hrubis <metan@ucw.cz>
 */

/*

  Convolution filters.

 */

#ifndef FILTERS_GP_CONVOLUTION_H
#define FILTERS_GP_CONVOLUTION_H

#include <filters/gp_filter.h>
#include <filters/gp_linear.h>

/*
 * 2D convolution kernel.
 *
 * The kernel array size must be w * h.
 *
 * The div is used to divide the resulting value which is commonly used for
 * normalization.
 *
 * _example box smoothing filter kernel initialization:
 *
 * float box_filter[] = {
 *	1, 1, 1,
 *	1, 1, 1,
 *	1, 1, 1,
 * };
 *
 * gp_filter_kernel_2d box_kernel = {
 *	.w = 3,
 *	.h = 3,
 *	.div = 9,
 *	.kernel = box_filter,
 * };
 */
typedef struct gp_filter_kernel_2d {
	unsigned int w;
	unsigned int h;
	float div;
	float *kernel;
} gp_filter_kernel_2d;

/*
 * _extended convolution filter.
 *
 * Works on rectangle in src defined by x_src, y_src, w_src and h_src.
 *
 * The result is stored into dst strating from x_dst and y_dst.
 *
 */
int gp_filter_convolution_ex(const gp_pixmap *src,
                             gp_coord x_src, gp_coord y_src,
                             gp_size w_src, gp_coord h_src,
                             gp_pixmap *dst,
                             gp_coord x_dst, gp_coord y_dst,
                             const gp_filter_kernel_2d *kernel,
                             gp_progress_cb *callback);

/*
 * _extended convolution filter.
 *
 * Works on rectangle in src defined by x_src, y_src, w_src and h_src.
 *
 * _allocates pixmap of a w_src x h_src.
 */
gp_pixmap *gp_filter_convolution_ex_alloc(const gp_pixmap *src,
                                          gp_coord x_src, gp_coord y_src,
                                          gp_size w_src, gp_size h_src,
                                          const gp_filter_kernel_2d *kernel,
                                          gp_progress_cb *callback);


static inline int gp_filter_convolution(const gp_pixmap *src, gp_pixmap *dst,
                                        const gp_filter_kernel_2d *kernel,
                                        gp_progress_cb *callback)
{
	return gp_filter_convolution_ex(src, 0, 0, dst->w, dst->h, dst, 0, 0,
	                                kernel, callback);
}

static inline gp_pixmap *gp_filter_convolution_alloc(const gp_pixmap *src,
                                                     const gp_filter_kernel_2d *kernel,
                                                     gp_progress_cb *callback)
{
	return gp_filter_convolution_ex_alloc(src, 0, 0, src->w, src->h,
                                              kernel, callback);
}

/*
 * Prints a kernel into the stdout.
 */
static inline void gp_filter_kernel_2d_print(const gp_filter_kernel_2d *kernel)
{
	gp_filter_kernel_print_raw(kernel->kernel, kernel->w, kernel->h,
	                           kernel->div);
}

#endif /* FILTERS_GP_CONVOLUTION_H */
