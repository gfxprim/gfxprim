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
  
   Sigma Lee filter.

   The xrad and yrad denotes radius the filter works on. The number of neighbor
   pixels is exactly 2 * rad + 1 for both directions.
   
   The sigma denotes maximal symetric difference of pixels scaled to [0,1]
   interval. Greater sigma causes results to be closer to mean linear filter. 

   The min parameter defines minimial number of pixels that must be in the two
   sigma iterval, if there is a less pixels in this interval the new pixel
   value is computed as mean of the surrounding pixels (not including the
   center one).

  */

#ifndef GP_FILTERS_SIGMA_H
#define GP_FILTERS_SIGMA_H

#include "GP_Filter.h"

int GP_FilterSigmaEx(const GP_Context *src,
                      GP_Coord x_src, GP_Coord y_src,
                      GP_Size w_src, GP_Size h_src,
                      GP_Context *dst,
                      GP_Coord x_dst, GP_Coord y_dst,
                      int xrad, int yrad,
                      unsigned int min, float sigma,
                      GP_ProgressCallback *callback);

GP_Context *GP_FilterSigmaExAlloc(const GP_Context *src,
                                   GP_Coord x_src, GP_Coord y_src,
                                   GP_Size w_src, GP_Size h_src,
                                   int xrad, int yrad,
                                   unsigned int min, float sigma,
                                   GP_ProgressCallback *callback);

static inline int GP_FilterSigma(const GP_Context *src,
                                  GP_Context *dst,
                                  int xrad, int yrad,
                                  unsigned int min, float sigma,
                                  GP_ProgressCallback *callback)
{
	return GP_FilterSigmaEx(src, 0, 0, src->w, src->h,
	                        dst, 0, 0, xrad, yrad, min, sigma, callback);
}

static inline GP_Context *GP_FilterSigmaAlloc(const GP_Context *src,
                                              int xrad, int yrad,
                                              unsigned int min, float sigma,
                                              GP_ProgressCallback *callback)
{
	return GP_FilterSigmaExAlloc(src, 0, 0, src->w, src->h,
	                             xrad, yrad, min, sigma, callback);
}

#endif /* GP_FILTERS_SIGMA_H */
