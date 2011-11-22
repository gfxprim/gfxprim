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
 * Copyright (C) 2009-2011 Cyril Hrubis <metan@ucw.cz>                       *
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
 * Increments each pixel channel by a given value.
 */
int GP_FilterBrightness_Raw(const GP_Context *src, GP_Context *dst,
                            GP_FilterParam params[],
                            GP_ProgressCallback *callback);

GP_Context *GP_FilterBrightness(const GP_Context *src, GP_Context *dst,
                                GP_FilterParam params[],
				GP_ProgressCallback *callback);

/*
 * Contrast filter.
 *
 * Multiplies each pixel channel by a given float value.
 *
 * The parameters should have the same pixel channels as
 * source pixel type and are expected to be float numbers.
 */
int GP_FilterContrast_Raw(const GP_Context *src, GP_Context *dst,
                          GP_FilterParam params[],
			  GP_ProgressCallback *callback);

GP_Context *GP_FilterContrast(const GP_Context *src, GP_Context *dst, 
                              GP_FilterParam params[],
			      GP_ProgressCallback *callback);

/*
 * Invert filter.
 *
 * Inverts each pixel channel (eg. val = max - val)
 */
int GP_FilterInvert_Raw(const GP_Context *src, GP_Context *dst,
                        GP_ProgressCallback *callback);

GP_Context *GP_FilterInvert(const GP_Context *src, GP_Context *dst,
                            GP_ProgressCallback *callback);

/*
 * Generic slow point filter.
 *
 * The filter_callback[] is expected to be filled with pointers
 * to functions of type uint32_t (*func)(uint32_t chan_val, uint8_t chan_size, GP_FilterParam *priv)
 *
 * The priv[] is free for your use and corresponding
 */
GP_Context *GP_FilterPoint(const GP_Context *src, GP_Context *dst,
                           GP_FilterParam filter_callback[],
			   GP_FilterParam priv[],
			   GP_ProgressCallback *callback);

#endif /* FILTERS_GP_POINT_H */
