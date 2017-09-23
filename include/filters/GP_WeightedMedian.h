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
 * Copyright (C) 2009-2012 Cyril Hrubis <metan@ucw.cz>                       *
 *                                                                           *
 *****************************************************************************/

 /*


  */

#ifndef FILTERS_GP_WEIGHTED_MEDIAN_H
#define FILTERS_GP_WEIGHTED_MEDIAN_H

#include "GP_Filter.h"

typedef struct GP_MedianWeights {
	unsigned int w;
	unsigned int h;
	unsigned int *weights;
} GP_MedianWeights;

int GP_FilterWeightedMedianEx(const GP_Pixmap *src,
                              GP_Coord x_src, GP_Coord y_src,
                              GP_Size w_src, GP_Size h_src,
                              GP_Pixmap *dst,
                              GP_Coord x_dst, GP_Coord y_dst,
                              GP_MedianWeights *weights,
                              GP_ProgressCallback *callback);

GP_Pixmap *GP_FilterWeightedMedianExAlloc(const GP_Pixmap *src,
                                           GP_Coord x_src, GP_Coord y_src,
                                           GP_Size w_src, GP_Size h_src,
                                           GP_MedianWeights *weights,
                                           GP_ProgressCallback *callback);

static inline int GP_FilterWeightedMedian(const GP_Pixmap *src,
                                          GP_Pixmap *dst,
                                          GP_MedianWeights *weights,
                                          GP_ProgressCallback *callback)
{
	return GP_FilterWeightedMedianEx(src, 0, 0, src->w, src->h,
	                                 dst, 0, 0, weights, callback);
}

static inline GP_Pixmap *GP_FilterWeightedMedianAlloc(const GP_Pixmap *src,
                                                       GP_MedianWeights *weights,
                                                       GP_ProgressCallback *callback)
{
	return GP_FilterWeightedMedianExAlloc(src, 0, 0, src->w, src->h,
	                                      weights, callback);
}

#endif /* FILTERS_GP_WEIGHTED_MEDIAN_H */
