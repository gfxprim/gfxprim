/*****************************************************************************
 * This file is part of gfxprim library.                                     *
 *                                                                           *
 * Gfxprim is free software; you can redistribute it and/or                  *
 * modify it under the terms of the GNU Lesser General Public                *
 * License as published by the Free Software Foundation; either              *
 * version 2.1 of the License, or (at your option) any later version.        *
 *                                                                           *
 * Gfxprim is distributed in the hope that it will be useful,                *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of            *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU         *
 * Lesser General Public License for more details.                           *
 *                                                                           *
 * You should have received a copy of the GNU Lesser General Public          *
 * License along with gfxprim; if not, write to the Free Software            *
 * Foundation, Inc., 51 Franklin Street, Fifth Floor,                        *
 * Boston, MA  02110-1301  USA                                               *
 *                                                                           *
 * Copyright (C) 2009-2013 Cyril Hrubis <metan@ucw.cz>                       *
 *                                                                           *
 *****************************************************************************/

/*

  Point filters, these works on individual pixels.

 */

#ifndef FILTERS_GP_POINT_H
#define FILTERS_GP_POINT_H

#include <filters/GP_Filter.h>

/*
 * Brightness filter.
 *
 * Increments each pixel channel by a p * channel_max value.
 */
int gp_filter_brightness_ex(const gp_pixmap *const src,
                            gp_coord x_src, gp_coord y_src,
                            gp_size w_src, gp_size h_src,
                            gp_pixmap *dst,
                            gp_coord x_dst, gp_coord y_dst,
                            float p,
                            gp_progress_cb *callback);

gp_pixmap *gp_filter_brightness_ex_alloc(const gp_pixmap *const src,
                                         gp_coord x_src, gp_coord y_src,
                                         gp_size w_src, gp_size h_src,
                                         float p,
                                         gp_progress_cb *callback);

static inline int gp_filter_brightness(const gp_pixmap *src, gp_pixmap *dst,
                                       float p, gp_progress_cb *callback)
{
	return gp_filter_brightness_ex(src, 0, 0, src->w, src->h,
	                               dst, 0, 0, p, callback);
}

static inline gp_pixmap * gp_filter_brightness_alloc(const gp_pixmap *src,
                                                     float p,
                                                     gp_progress_cb *callback)
{
	return gp_filter_brightness_ex_alloc(src, 0, 0, src->w, src->h,
	                                     p, callback);
}

/*
 * Contrast filter.
 *
 * Multiplies each pixel channel by a given float value.
 *
 * The parameters should have the same pixel channels as
 * source pixel type and are expected to be float numbers.
 */
int gp_filter_contrast_ex(const gp_pixmap *const src,
                          gp_coord x_src, gp_coord y_src,
                          gp_size w_src, gp_size h_src,
                          gp_pixmap *dst,
                          gp_coord x_dst, gp_coord y_dst,
                          float p,
                          gp_progress_cb *callback);

gp_pixmap *gp_filter_contrast_ex_alloc(const gp_pixmap *const src,
                                       gp_coord x_src, gp_coord y_src,
                                       gp_size w_src, gp_size h_src,
                                       float p,
                                       gp_progress_cb *callback);

static inline int gp_filter_contrast(const gp_pixmap *src, gp_pixmap *dst,
                                     float p, gp_progress_cb *callback)
{
	return gp_filter_contrast_ex(src, 0, 0, src->w, src->h,
	                             dst, 0, 0, p, callback);
}

static inline gp_pixmap *gp_filter_contrast_alloc(const gp_pixmap *src,
                                                  float p,
                                                  gp_progress_cb *callback)
{
	return gp_filter_contrast_ex_alloc(src, 0, 0, src->w, src->h,
	                                   p, callback);
}

/*
 * Brightness and Contrast combined.
 */
int gp_filter_brightness_contrast_ex(const gp_pixmap *const src,
                                     gp_coord x_src, gp_coord y_src,
                                     gp_size w_src, gp_size h_src,
                                     gp_pixmap *dst,
                                     gp_coord x_dst, gp_coord y_dst,
                                     float b, float c,
                                     gp_progress_cb *callback);

gp_pixmap *gp_filter_brightness_contrast_ex_alloc(const gp_pixmap *const src,
                                                  gp_coord x_src, gp_coord y_src,
                                                  gp_size w_src, gp_size h_src,
                                                  float b, float c,
                                                  gp_progress_cb *callback);
static inline int
gp_filter_brightness_contrast(const gp_pixmap *src, gp_pixmap *dst,
                            float b, float c, gp_progress_cb *callback)
{
	return gp_filter_brightness_contrast_ex(src, 0, 0, src->w, src->h,
	                                        dst, 0, 0, b, c, callback);
}

static inline gp_pixmap *
gp_filter_brightness_contrast_alloc(const gp_pixmap *src,
                                    float b, float c,
                                    gp_progress_cb *callback)
{
	return gp_filter_brightness_contrast_ex_alloc(src, 0, 0, src->w, src->h,
	                                              b, c, callback);
}

/*
 * Posterize
 *
 * Does quantization into steps regions.
 */
int gp_filter_posterize_ex(const gp_pixmap *const src,
                           gp_coord x_src, gp_coord y_src,
                           gp_size w_src, gp_size h_src,
                           gp_pixmap *dst,
                           gp_coord x_dst, gp_coord y_dst,
                           unsigned int steps,
                           gp_progress_cb *callback);

gp_pixmap *gp_filter_posterize_ex_alloc(const gp_pixmap *const src,
                                        gp_coord x_src, gp_coord y_src,
                                        gp_size w_src, gp_size h_src,
                                        unsigned int steps,
                                        gp_progress_cb *callback);

static inline int gp_filter_posterize(const gp_pixmap *src, gp_pixmap *dst,
                                     unsigned int steps,
                                     gp_progress_cb *callback)
{
	return gp_filter_posterize_ex(src, 0, 0, src->w, src->h,
	                              dst, 0, 0, steps, callback);
}

static inline gp_pixmap *
gp_filter_posterize_alloc(const gp_pixmap *src, unsigned int steps,
                          gp_progress_cb *callback)
{
	return gp_filter_posterize_ex_alloc(src, 0, 0, src->w, src->h,
	                                    steps, callback);
}

/*
 * Inverts the pixel value, i.e. sets it to max - val.
 */
int gp_filter_invert_ex(const gp_pixmap *const src,
                        gp_coord x_src, gp_coord y_src,
                        gp_size w_src, gp_size h_src,
                        gp_pixmap *dst,
                        gp_coord x_dst, gp_coord y_dst,
                        gp_progress_cb *callback);

gp_pixmap *gp_filter_invert_ex_alloc(const gp_pixmap *const src,
                                     gp_coord x_src, gp_coord y_src,
                                     gp_size w_src, gp_size h_src,
                                     gp_progress_cb *callback);

static inline int gp_filter_invert(const gp_pixmap *src, gp_pixmap *dst,
                                   gp_progress_cb *callback)
{
	return gp_filter_invert_ex(src, 0, 0, src->w, src->h,
	                           dst, 0, 0, callback);
}

static inline gp_pixmap *gp_filter_invert_alloc(const gp_pixmap *src,
                                                gp_progress_cb *callback)
{
	return gp_filter_invert_ex_alloc(src, 0, 0, src->w, src->h, callback);
}

#endif /* FILTERS_GP_POINT_H */
