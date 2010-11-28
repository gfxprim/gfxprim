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

#include "GP.h"

#include <stdlib.h>

/* Generate drawing functions for various bit depths. */
#define CONTEXT_T GP_Context *
#define PIXVAL_T  GP_Pixel
	#define FN_NAME GP_FillTriangle8bpp
	#define LINE GP_Line8bpp
	#define HLINE GP_HLine8bpp
		#include "algo/FillTriangle.algo.c"
	#undef HLINE
	#undef LINE
	#undef FN_NAME
	#define FN_NAME GP_FillTriangle16bpp
	#define LINE GP_Line16bpp
	#define HLINE GP_HLine16bpp
		#include "algo/FillTriangle.algo.c"
	#undef HLINE
	#undef LINE
	#undef FN_NAME
	#define FN_NAME GP_FillTriangle24bpp
	#define LINE GP_Line24bpp
	#define HLINE GP_HLine24bpp
		#include "algo/FillTriangle.algo.c"
	#undef HLINE
	#undef LINE
	#undef FN_NAME
	#define FN_NAME GP_FillTriangle32bpp
	#define LINE GP_Line32bpp
	#define HLINE GP_HLine32bpp
		#include "algo/FillTriangle.algo.c"
	#undef HLINE
	#undef LINE
	#undef FN_NAME
#undef CONTEXT_T
#undef PIXVAL_T

GP_RetCode GP_FillTriangle(GP_Context * context, int x0, int y0, int x1, int y1,
                           int x2, int y2, GP_Color color)
{
	GP_CHECK_CONTEXT(context);

	GP_Pixel pixel;
	pixel.type = context->pixel_type;
	GP_RetCode ret = GP_ColorToPixel(color, &pixel);

	switch (context->bits_per_pixel) {
	case 8:
		GP_FillTriangle8bpp(context, x0, y0, x1, y1, x2, y2, pixel);
		break;
	case 16:
		GP_FillTriangle16bpp(context, x0, y0, x1, y1, x2, y2, pixel);
		break;
	case 24:
		GP_FillTriangle24bpp(context, x0, y0, x1, y1, x2, y2, pixel);
		break;
	case 32:
		GP_FillTriangle32bpp(context, x0, y0, x1, y1, x2, y2, pixel);
		break;
	default:
		return GP_ENOIMPL;
	}

	return ret;
}

GP_RetCode GP_TFillTriangle(GP_Context* context, int x0, int y0, int x1, int y1,
                            int x2, int y2, GP_Color color)
{
	GP_TRANSFORM_POINT(context, x0, y0);
	GP_TRANSFORM_POINT(context, x1, y1);
	GP_TRANSFORM_POINT(context, x2, y2);
	return GP_FillTriangle(context, x0, y0, x1, y1, x2, y2, color);
}
