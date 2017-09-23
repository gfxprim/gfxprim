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

#include "core/GP_GetPutPixel.h"
#include "core/GP_FnPerBpp.h"

#include "gfx/GP_VLineAA.h"
#include "gfx/GP_HLineAA.h"

/*
void GP_VLineXYY_Raw(GP_Pixmap *pixmap, GP_Coord x, GP_Coord y0,
                     GP_Coord y1, GP_Pixel pixel)
{
	GP_CHECK_PIXMAP(pixmap);

	GP_FN_PER_BPP_PIXMAP(GP_VLine, pixmap, pixmap, x, y0, y1, pixel);
}
*/

void GP_VLineAA(GP_Pixmap *pixmap, GP_Coord x, GP_Coord y0,
                GP_Coord y1, GP_Pixel pixel)
{
	GP_CHECK_PIXMAP(pixmap);

	if (pixmap->axes_swap) {
		GP_TRANSFORM_Y_FP(pixmap, x);
		GP_TRANSFORM_X_FP(pixmap, y0);
		GP_TRANSFORM_X_FP(pixmap, y1);
		GP_HLineAA_Raw(pixmap, y0, y1, x, pixel);
	} else {
		GP_TRANSFORM_X_FP(pixmap, x);
		GP_TRANSFORM_Y_FP(pixmap, y0);
		GP_TRANSFORM_Y_FP(pixmap, y1);
		GP_VLineAA_Raw(pixmap, x, y0, y1, pixel);
	}
}
