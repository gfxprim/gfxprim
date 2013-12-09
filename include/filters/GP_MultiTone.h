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

#ifndef FILTERS_GP_MULTI_TONE_H
#define FILTERS_GP_MULTI_TONE_H

#include "GP_Filter.h"

int GP_FilterMultiToneEx(const GP_Context *const src,
                         GP_Coord x_src, GP_Coord y_src,
                         GP_Size w_src, GP_Size h_src,
                         GP_Context *dst,
                         GP_Coord x_dst, GP_Coord y_dst,
                         GP_Pixel pixels[], GP_Size pixels_size,
                         GP_ProgressCallback *callback);

static inline int GP_FilterMultiTone(const GP_Context *const src,
                                     GP_Context *dst,
                                     GP_Pixel pixels[], GP_Size pixels_size,
                                     GP_ProgressCallback *callback)
{
	return GP_FilterMultiToneEx(src, 0, 0, src->w, src->h, dst, 0, 0,
	                            pixels, pixels_size, callback);
}

GP_Context *GP_FilterMultiToneExAlloc(const GP_Context *const src,
                                      GP_Coord x_src, GP_Coord y_src,
                                      GP_Size w_src, GP_Size h_src,
                                      GP_PixelType dst_pixel_type,
                                      GP_Pixel pixels[], GP_Size pixels_size,
                                      GP_ProgressCallback *callback);

static inline GP_Context *GP_FilterMultiToneAlloc(const GP_Context *const src,
                                                  GP_PixelType dst_pixel_type,
                                                  GP_Pixel pixels[],
						  GP_Size pixels_size,
                                                  GP_ProgressCallback *callback)
{
	return GP_FilterMultiToneExAlloc(src, 0, 0, src->w, src->h,
	                                 dst_pixel_type,
	                                 pixels, pixels_size, callback);
}

#endif /* FILTERS_GP_MULTI_TONE_H */
