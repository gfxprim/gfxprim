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

#include "core/GP_Convert.h"
#include "core/GP_Debug.h"

#include "filters/GP_MultiTone.h"

#define PIX 3

static void init_sepia_tones(GP_Pixel pixels[PIX], GP_PixelType pixel_type)
{
//	pixels[0] = GP_RGBToPixel(56, 33, 10, pixel_type);
//	pixels[1] = GP_RGBToPixel(255, 255, 255, pixel_type);
	pixels[0] = GP_RGBToPixel(0, 0, 0, pixel_type);
	pixels[1] = GP_RGBToPixel(162, 116, 70, pixel_type);
	pixels[2] = GP_RGBToPixel(230, 230, 230, pixel_type);
}

int GP_FilterSepiaEx(const GP_Pixmap *const src,
                     GP_Coord x_src, GP_Coord y_src,
                     GP_Size w_src, GP_Size h_src,
                     GP_Pixmap *dst,
                     GP_Coord x_dst, GP_Coord y_dst,
                     GP_ProgressCallback *callback)
{
	GP_Pixel pixels[PIX];

	init_sepia_tones(pixels, dst->pixel_type);

	return GP_FilterMultiToneEx(src, x_src, y_src, w_src, h_src,
	                            dst, x_dst, y_dst, pixels, PIX, callback);
}

GP_Pixmap *GP_FilterSepiaExAlloc(const GP_Pixmap *const src,
                                  GP_Coord x_src, GP_Coord y_src,
                                  GP_Size w_src, GP_Size h_src,
                                  GP_PixelType dst_pixel_type,
                                  GP_ProgressCallback *callback)
{
	GP_Pixel pixels[PIX];

	init_sepia_tones(pixels, dst_pixel_type);

	return GP_FilterMultiToneExAlloc(src, x_src, y_src, w_src, h_src,
	                                 dst_pixel_type, pixels, PIX, callback);
}
