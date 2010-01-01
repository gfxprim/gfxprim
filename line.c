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

/*
 * Adaptive implementations of GP_Line(), GP_HLine() and GP_VLine().
 * They determine the bit depth of the underlying surface first,
 * and then call the appropriate specialized drawing function.
 */

void GP_Line(GP_TARGET_TYPE *target, long color, int x0, int y0, int x1, int y1)
{
	if (target == NULL || GP_PIXELS(target) == NULL)
		return;

	switch (GP_BYTES_PER_PIXEL(target)) {
	case 1:
		GP_Line_8bpp(target, color, x0, y0, x1, y1);
		break;
	case 2:
		GP_Line_16bpp(target, color, x0, y0, x1, y1);
		break;
	case 3:
		GP_Line_24bpp(target, color, x0, y0, x1, y1);
		break;
	case 4:
		GP_Line_32bpp(target, color, x0, y0, x1, y1);
		break;
	}
}

void GP_HLine(GP_TARGET_TYPE *target, long color, int x0, int x1, int y)
{
	if (target == NULL || GP_PIXELS(target) == NULL)
		return;

	switch (GP_BYTES_PER_PIXEL(target)) {
	case 1:
		GP_HLine_8bpp(target, color, x0, x1, y);
		break;
	case 2:
		GP_HLine_16bpp(target, color, x0, x1, y);
		break;
	case 3:
		GP_HLine_24bpp(target, color, x0, x1, y);
		break;
	case 4:
		GP_HLine_32bpp(target, color, x0, x1, y);
		break;
	}
}

void GP_VLine(GP_TARGET_TYPE *target, long color, int x, int y0, int y1)
{
	if (target == NULL || GP_PIXELS(target) == NULL)
		return;

	switch (GP_BYTES_PER_PIXEL(target)) {
	case 1:
		GP_VLine_8bpp(target, color, x, y0, y1);
		break;
	case 2:
		GP_VLine_16bpp(target, color, x, y0, y1);
		break;
	case 3:
		GP_VLine_24bpp(target, color, x, y0, y1);
		break;
	case 4:
		GP_VLine_32bpp(target, color, x, y0, y1);
		break;
	}
}

/*
 * Build specialized GP_Line() variants for various bit depths.
 */

#define FN_NAME		GP_Line_8bpp
#define SETPIXEL	GP_SetPixel_8bpp
#include "generic/line_generic.c"
#undef SETPIXEL
#undef FN_NAME

#define FN_NAME		GP_Line_16bpp
#define SETPIXEL	GP_SetPixel_16bpp
#include "generic/line_generic.c"
#undef SETPIXEL
#undef FN_NAME

#define FN_NAME		GP_Line_24bpp
#define SETPIXEL	GP_SetPixel_24bpp
#include "generic/line_generic.c"
#undef SETPIXEL
#undef FN_NAME

#define FN_NAME		GP_Line_32bpp
#define SETPIXEL	GP_SetPixel_32bpp
#include "generic/line_generic.c"
#undef SETPIXEL
#undef FN_NAME

/*
 * Build specialized GP_HLine() variants for various bit depths.
 */

#define FN_NAME		GP_HLine_8bpp
#define BYTES_PER_PIXEL	1
#define WRITE_PIXEL	GP_WRITE_PIXEL_1BYTE
#include "generic/hline_generic.c"
#undef WRITE_PIXEL
#undef BYTES_PER_PIXEL
#undef FN_NAME

#define FN_NAME		GP_HLine_16bpp
#define BYTES_PER_PIXEL	2
#define WRITE_PIXEL	GP_WRITE_PIXEL_2BYTES
#include "generic/hline_generic.c"
#undef WRITE_PIXEL
#undef BYTES_PER_PIXEL
#undef FN_NAME

#define FN_NAME		GP_HLine_24bpp
#define BYTES_PER_PIXEL	3
#define WRITE_PIXEL	GP_WRITE_PIXEL_3BYTES
#include "generic/hline_generic.c"
#undef WRITE_PIXEL
#undef BYTES_PER_PIXEL
#undef FN_NAME

#define FN_NAME		GP_HLine_32bpp
#define BYTES_PER_PIXEL	4
#define WRITE_PIXEL	GP_WRITE_PIXEL_4BYTES
#include "generic/hline_generic.c"
#undef WRITE_PIXEL
#undef BYTES_PER_PIXEL
#undef FN_NAME

/*
 * Build specialized GP_VLine() variants for various bit depths.
 */

#define FN_NAME		GP_VLine_8bpp
#define WRITE_PIXEL	GP_WRITE_PIXEL_1BYTE
#include "generic/vline_generic.c"
#undef WRITE_PIXEL
#undef FN_NAME

#define FN_NAME		GP_VLine_16bpp
#define WRITE_PIXEL	GP_WRITE_PIXEL_2BYTES
#include "generic/vline_generic.c"
#undef WRITE_PIXEL
#undef FN_NAME

#define FN_NAME		GP_VLine_24bpp
#define WRITE_PIXEL	GP_WRITE_PIXEL_3BYTES
#include "generic/vline_generic.c"
#undef WRITE_PIXEL
#undef FN_NAME

#define FN_NAME		GP_VLine_32bpp
#define WRITE_PIXEL	GP_WRITE_PIXEL_4BYTES
#include "generic/vline_generic.c"
#undef WRITE_PIXEL
#undef FN_NAME

void GP_HLineWide(SDL_Surface *surf, long color, enum GP_LinePosition pos, uint8_t thickness, int x0, int x1, int y)
{
	switch (pos) {
		case GP_LINE_ABOVE:
			GP_FillRect(surf, color, x0, y + thickness, x1, y);
		break;
		case GP_LINE_BELOW:
			GP_FillRect(surf, color, x0, y - thickness, x1, y);
		break;
		case GP_LINE_CENTER:
			GP_FillRect(surf, color, x0, y - thickness, x1, y +  thickness);
		break;
	}
}

void GP_VLineWide(SDL_Surface *surf, long color, enum GP_LinePosition pos, uint8_t thickness, int x, int y0, int y1)
{
	switch (pos) {
		case GP_LINE_ABOVE:
			GP_FillRect(surf, color, x + thickness, y0, x, y1);
		break;
		case GP_LINE_BELOW:
			GP_FillRect(surf, color, x - thickness, y0, x, y1);
		break;
		case GP_LINE_CENTER:
			GP_FillRect(surf, color, x - thickness, y0, x + thickness, y1);
		break;
	}
}

