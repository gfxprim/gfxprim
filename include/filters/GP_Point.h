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
int GP_FilterBrightnessEx(const GP_FilterArea *area, float p,
                          GP_ProgressCallback *callback);

GP_Context *GP_FilterBrightnessExAlloc(const GP_FilterArea *area, float p,
                                       GP_ProgressCallback *callback);

static inline int GP_FilterBrightness(const GP_Context *src, GP_Context *dst,
                                      float p, GP_ProgressCallback *callback)
{
	GP_FILTER_AREA_DEFAULT(src, dst);

	return GP_FilterBrightnessEx(&area, p, callback);
}

static inline GP_Context *
GP_FilterBrightnessAlloc(const GP_Context *src, float p,
                         GP_ProgressCallback *callback)
{
	GP_FILTER_AREA_DEFAULT(src, NULL);

	return GP_FilterBrightnessExAlloc(&area, p, callback);
}

/*
 * Contrast filter.
 *
 * Multiplies each pixel channel by a given float value.
 *
 * The parameters should have the same pixel channels as
 * source pixel type and are expected to be float numbers.
 */
int GP_FilterContrastEx(const GP_FilterArea *area, float p,
                        GP_ProgressCallback *callback);

GP_Context *GP_FilterContrastExAlloc(const GP_FilterArea *area, float p,
                                     GP_ProgressCallback *callback);

static inline int GP_FilterContrast(const GP_Context *src, GP_Context *dst,
                                    float p, GP_ProgressCallback *callback)
{
	GP_FILTER_AREA_DEFAULT(src, dst);

	return GP_FilterContrastEx(&area, p, callback);
}

static inline GP_Context *GP_FilterContrastAlloc(const GP_Context *src,
                                                 float p,
                                                 GP_ProgressCallback *callback)
{
	GP_FILTER_AREA_DEFAULT(src, NULL);

	return GP_FilterContrastExAlloc(&area, p, callback);
}

/*
 * Brightness and Contrast combined.
 */
int GP_FilterBrightnessContrastEx(const GP_FilterArea *area, float b, float c,
                                  GP_ProgressCallback *callback);

GP_Context *GP_FilterBrightnessContrastExAlloc(const GP_FilterArea *area,
                                               float b, float c,
                                               GP_ProgressCallback *callback);

static inline int
GP_FilterBrightnessContrast(const GP_Context *src, GP_Context *dst,
                            float b, float c, GP_ProgressCallback *callback)
{
	GP_FILTER_AREA_DEFAULT(src, dst);

	return GP_FilterBrightnessContrastEx(&area, b, c, callback);
}

static inline GP_Context *
GP_FilterBrightnessContrastAlloc(const GP_Context *src,
                                 float b, float c,
                                 GP_ProgressCallback *callback)
{
	GP_FILTER_AREA_DEFAULT(src, NULL);

	return GP_FilterBrightnessContrastExAlloc(&area, b, c, callback);
}

/*
 * Posterize
 *
 * Does quantization into steps regions.
 */
int GP_FilterPosterizeEx(const GP_FilterArea *area, unsigned int steps,
                         GP_ProgressCallback *callback);

GP_Context *GP_FilterPosterizeExAlloc(const GP_FilterArea *area,
                                      unsigned int steps,
                                      GP_ProgressCallback *callback);

static inline int GP_FilterPosterize(const GP_Context *src, GP_Context *dst,
                                     unsigned int steps,
                                     GP_ProgressCallback *callback)
{
	GP_FILTER_AREA_DEFAULT(src, dst);

	return GP_FilterPosterizeEx(&area, steps, callback);
}

static inline GP_Context *
GP_FilterPosterizeAlloc(const GP_Context *src, unsigned int steps,
                        GP_ProgressCallback *callback)
{
	GP_FILTER_AREA_DEFAULT(src, NULL);

	return GP_FilterPosterizeExAlloc(&area, steps, callback);
}

/*
 * Inverts the pixel value, i.e. sets it to max - val.
 */
int GP_FilterInvertEx(const GP_FilterArea *area,
                      GP_ProgressCallback *callback);

GP_Context *GP_FilterInvertExAlloc(const GP_FilterArea *area,
                                   GP_ProgressCallback *callback);

static inline int GP_FilterInvert(const GP_Context *src, GP_Context *dst,
                                  GP_ProgressCallback *callback)
{
	GP_FILTER_AREA_DEFAULT(src, dst);

	return GP_FilterInvertEx(&area, callback);
}

static inline GP_Context *GP_FilterInvertAlloc(const GP_Context *src,
                                               GP_ProgressCallback *callback)
{
	GP_FILTER_AREA_DEFAULT(src, NULL);

	return GP_FilterInvertExAlloc(&area, callback);
}

#endif /* FILTERS_GP_POINT_H */
