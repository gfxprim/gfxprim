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
#include "GP_line.h"
#include "GP_gfx.h"

void GP_Triangle(GP_TARGET_TYPE *target, long color, int x0, int y0, int x1, int y1, int x2, int y2)
{
	if (target == NULL || GP_PIXELS(target) == NULL)
		return;

	switch (GP_BYTES_PER_PIXEL(target)) {
	case 1:
		GP_Triangle_8bpp(target, color, x0, y0, x1, y1, x2, y2);
		break;
	case 2:
		GP_Triangle_16bpp(target, color, x0, y0, x1, y1, x2, y2);
		break;
	case 3:
		GP_Triangle_24bpp(target, color, x0, y0, x1, y1, x2, y2);
		break;
	case 4:
		GP_Triangle_32bpp(target, color, x0, y0, x1, y1, x2, y2);
		break;
	}
}

void GP_FillTriangle(GP_TARGET_TYPE *target, long color, int x0, int y0, int x1, int y1, int x2, int y2)
{
	if (target == NULL || GP_PIXELS(target) == NULL)
		return;

	switch (GP_BYTES_PER_PIXEL(target)) {
	case 1:
		GP_FillTriangle_8bpp(target, color, x0, y0, x1, y1, x2, y2);
		break;
	case 2:
		GP_FillTriangle_16bpp(target, color, x0, y0, x1, y1, x2, y2);
		break;
	case 3:
		GP_FillTriangle_24bpp(target, color, x0, y0, x1, y1, x2, y2);
		break;
	case 4:
		GP_FillTriangle_32bpp(target, color, x0, y0, x1, y1, x2, y2);
		break;
	}
}

/* Build specializations of GP_Triangle() for various bit depths */

#define FN_NAME		GP_Triangle_8bpp
#define SETPIXEL	GP_SetPixel_8bpp
#include "generic/triangle_generic.c"
#undef SETPIXEL
#undef FN_NAME

#define FN_NAME		GP_Triangle_16bpp
#define SETPIXEL	GP_SetPixel_16bpp
#include "generic/triangle_generic.c"
#undef SETPIXEL
#undef FN_NAME

#define FN_NAME		GP_Triangle_24bpp
#define SETPIXEL	GP_SetPixel_24bpp
#include "generic/triangle_generic.c"
#undef SETPIXEL
#undef FN_NAME

#define FN_NAME		GP_Triangle_32bpp
#define SETPIXEL	GP_SetPixel_32bpp
#include "generic/triangle_generic.c"
#undef SETPIXEL
#undef FN_NAME

/* Build specializations of GP_FillTriangle() for various bit depths */

#define FN_NAME		GP_FillTriangle_8bpp
#define SETPIXEL	GP_SetPixel_8bpp
#include "generic/fill_triangle_generic.c"
#undef SETPIXEL
#undef FN_NAME

#define FN_NAME		GP_FillTriangle_16bpp
#define SETPIXEL	GP_SetPixel_16bpp
#include "generic/fill_triangle_generic.c"
#undef SETPIXEL
#undef FN_NAME

#define FN_NAME		GP_FillTriangle_24bpp
#define SETPIXEL	GP_SetPixel_24bpp
#include "generic/fill_triangle_generic.c"
#undef SETPIXEL
#undef FN_NAME

#define FN_NAME		GP_FillTriangle_32bpp
#define SETPIXEL	GP_SetPixel_32bpp
#include "generic/fill_triangle_generic.c"
#undef SETPIXEL
#undef FN_NAME

