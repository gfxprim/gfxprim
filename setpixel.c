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
 * Copyright (C) 2009-2010 Jiri "BlueBear" Dluhos                            *
 *                         <jiri.bluebear.dluhos@gmail.com>                  *
 *                                                                           *
 * Copyright (C) 2009-2010 Cyril Hrubis <metan@ucw.cz>                       *
 *                                                                           *
 *****************************************************************************/

#include "GP_backend.h"
#include "GP_pixel.h"

/*
 * An adaptive implementation of GP_SetPixel(); it automatically identifies
 * the number of bits per pixel of the image and then uses the appropriate
 * pixel writing routine.
 */
void GP_SetPixel(GP_TARGET_TYPE *target, GP_COLOR_TYPE color, int x, int y)
{
	if (target == NULL || GP_PIXELS(target) == NULL)
		return;

	int bytes_per_pixel = GP_BYTES_PER_PIXEL(target);

	/* Clip coordinates against the clip rectangle of the surface */
	int xmin, xmax, ymin, ymax;
	GP_GET_CLIP_RECT(target, xmin, xmax, ymin, ymax);

	if (x < xmin || y < ymin || x > xmax || y > ymax)
		return;

	/* Compute the address of the pixel */
	uint8_t *p = GP_PIXEL_ADDR(target, x, y);

	switch (bytes_per_pixel) {
	case 1:
		GP_WRITE_PIXEL_1BYTE(p, color);
		break;
	
	case 2:
		GP_WRITE_PIXEL_2BYTES(p, color);
		break;

	case 3:
		GP_WRITE_PIXEL_3BYTES(p, color);
		break;

	case 4:
		GP_WRITE_PIXEL_4BYTES(p, color);
		break;
	}
}

/*
 * Build specializations of GP_SetPixel() for various bit depths.
 */

#define FN_NAME		GP_SetPixel_8bpp
#define WRITE_PIXEL	GP_WRITE_PIXEL_1BYTE
#include "generic/setpixel_generic.c"
#undef FN_NAME
#undef WRITE_PIXEL

#define FN_NAME		GP_SetPixel_16bpp
#define WRITE_PIXEL	GP_WRITE_PIXEL_2BYTES
#include "generic/setpixel_generic.c"
#undef FN_NAME
#undef WRITE_PIXEL

#define FN_NAME		GP_SetPixel_24bpp
#define WRITE_PIXEL	GP_WRITE_PIXEL_3BYTES
#include "generic/setpixel_generic.c"
#undef FN_NAME
#undef WRITE_PIXEL

#define FN_NAME		GP_SetPixel_32bpp
#define WRITE_PIXEL	GP_WRITE_PIXEL_4BYTES
#include "generic/setpixel_generic.c"
#undef FN_NAME
#undef WRITE_PIXEL

/*
 * Build unsafe variants of GP_SetPixel().
 */

#define NONULL 1
#define NOCLIP 1

#define FN_NAME		GP_SetPixel_8bpp_unsafe
#define WRITE_PIXEL	GP_WRITE_PIXEL_1BYTE
#include "generic/setpixel_generic.c"
#undef FN_NAME
#undef WRITE_PIXEL

#define FN_NAME		GP_SetPixel_16bpp_unsafe
#define WRITE_PIXEL	GP_WRITE_PIXEL_2BYTES
#include "generic/setpixel_generic.c"
#undef FN_NAME
#undef WRITE_PIXEL

#define FN_NAME		GP_SetPixel_24bpp_unsafe
#define WRITE_PIXEL	GP_WRITE_PIXEL_3BYTES
#include "generic/setpixel_generic.c"
#undef FN_NAME
#undef WRITE_PIXEL

#define FN_NAME		GP_SetPixel_32bpp_unsafe
#define WRITE_PIXEL	GP_WRITE_PIXEL_4BYTES
#include "generic/setpixel_generic.c"
#undef FN_NAME
#undef WRITE_PIXEL

#undef NONULL
#undef NOCLIP

