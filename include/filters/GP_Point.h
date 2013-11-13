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

#include "GP_Filter.h"

/*
 * Brightness filter.
 *
 * Increments each pixel channel by a p * channel_max value.
 */
int GP_FilterBrightnessEx(const GP_Context *const src,
                          GP_Coord x_src, GP_Coord y_src,
                          GP_Size w_src, GP_Size h_src,
                          GP_Context *dst,
                          GP_Coord x_dst, GP_Coord y_dst,
                          float p,
                          GP_ProgressCallback *callback);

GP_Context *GP_FilterBrightnessExAlloc(const GP_Context *const src,
                                       GP_Coord x_src, GP_Coord y_src,
                                       GP_Size w_src, GP_Size h_src,
                                       float p,
                                       GP_ProgressCallback *callback);

static inline int GP_FilterBrightness(const GP_Context *src, GP_Context *dst,
                                      float p, GP_ProgressCallback *callback)
{
	return GP_FilterBrightnessEx(src, 0, 0, src->w, src->h,
	                             dst, 0, 0, p, callback);
}

static inline GP_Context *
GP_FilterBrightnessAlloc(const GP_Context *src, float p,
                         GP_ProgressCallback *callback)
{
	return GP_FilterBrightnessExAlloc(src, 0, 0, src->w, src->h,
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
int GP_FilterContrastEx(const GP_Context *const src,
                        GP_Coord x_src, GP_Coord y_src,
                        GP_Size w_src, GP_Size h_src,
                        GP_Context *dst,
                        GP_Coord x_dst, GP_Coord y_dst,
                        float p,
                        GP_ProgressCallback *callback);

GP_Context *GP_FilterContrastExAlloc(const GP_Context *const src,
                                     GP_Coord x_src, GP_Coord y_src,
                                     GP_Size w_src, GP_Size h_src,
                                     float p,
                                     GP_ProgressCallback *callback);

static inline int GP_FilterContrast(const GP_Context *src, GP_Context *dst,
                                    float p, GP_ProgressCallback *callback)
{
	return GP_FilterContrastEx(src, 0, 0, src->w, src->h,
	                           dst, 0, 0, p, callback);
}

static inline GP_Context *GP_FilterContrastAlloc(const GP_Context *src,
                                                 float p,
                                                 GP_ProgressCallback *callback)
{
	return GP_FilterContrastExAlloc(src, 0, 0, src->w, src->h,
	                                p, callback);
}

/*
 * Brightness and Contrast combined.
 */
int GP_FilterBrightnessContrastEx(const GP_Context *const src,
                                  GP_Coord x_src, GP_Coord y_src,
                                  GP_Size w_src, GP_Size h_src,
                                  GP_Context *dst,
                                  GP_Coord x_dst, GP_Coord y_dst,
                                  float b, float c,
                                  GP_ProgressCallback *callback);

GP_Context *GP_FilterBrightnessContrastExAlloc(const GP_Context *const src,
                                               GP_Coord x_src, GP_Coord y_src,
                                               GP_Size w_src, GP_Size h_src,
                                               float b, float c,
                                               GP_ProgressCallback *callback);
static inline int
GP_FilterBrightnessContrast(const GP_Context *src, GP_Context *dst,
                            float b, float c, GP_ProgressCallback *callback)
{
	return GP_FilterBrightnessContrastEx(src, 0, 0, src->w, src->h,
	                                     dst, 0, 0, b, c, callback);
}

static inline GP_Context *
GP_FilterBrightnessContrastAlloc(const GP_Context *src,
                                 float b, float c,
                                 GP_ProgressCallback *callback)
{
	return GP_FilterBrightnessContrastExAlloc(src, 0, 0, src->w, src->h,
	                                          b, c, callback);
}

/*
 * Posterize
 *
 * Does quantization into steps regions.
 */
int GP_FilterPosterizeEx(const GP_Context *const src,
                         GP_Coord x_src, GP_Coord y_src,
                         GP_Size w_src, GP_Size h_src,
                         GP_Context *dst,
                         GP_Coord x_dst, GP_Coord y_dst,
                         unsigned int steps,
                         GP_ProgressCallback *callback);

GP_Context *GP_FilterPosterizeExAlloc(const GP_Context *const src,
                                      GP_Coord x_src, GP_Coord y_src,
                                      GP_Size w_src, GP_Size h_src,
                                      unsigned int steps,
                                      GP_ProgressCallback *callback);

static inline int GP_FilterPosterize(const GP_Context *src, GP_Context *dst,
                                     unsigned int steps,
                                     GP_ProgressCallback *callback)
{
	return GP_FilterPosterizeEx(src, 0, 0, src->w, src->h,
	                            dst, 0, 0, steps, callback);
}

static inline GP_Context *
GP_FilterPosterizeAlloc(const GP_Context *src, unsigned int steps,
                        GP_ProgressCallback *callback)
{
	return GP_FilterPosterizeExAlloc(src, 0, 0, src->w, src->h,
	                                 steps, callback);
}

/*
 * Inverts the pixel value, i.e. sets it to max - val.
 */
int GP_FilterInvertEx(const GP_Context *const src,
                      GP_Coord x_src, GP_Coord y_src,
                      GP_Size w_src, GP_Size h_src,
                      GP_Context *dst,
                      GP_Coord x_dst, GP_Coord y_dst,
                      GP_ProgressCallback *callback);

GP_Context *GP_FilterInvertExAlloc(const GP_Context *const src,
                                   GP_Coord x_src, GP_Coord y_src,
                                   GP_Size w_src, GP_Size h_src,
                                   GP_ProgressCallback *callback);

static inline int GP_FilterInvert(const GP_Context *src, GP_Context *dst,
                                  GP_ProgressCallback *callback)
{
	return GP_FilterInvertEx(src, 0, 0, src->w, src->h,
	                         dst, 0, 0, callback);
}

static inline GP_Context *GP_FilterInvertAlloc(const GP_Context *src,
                                               GP_ProgressCallback *callback)
{
	return GP_FilterInvertExAlloc(src, 0, 0, src->w, src->h, callback);
}

#endif /* FILTERS_GP_POINT_H */
