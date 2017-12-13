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

void gp_vline_aa(gp_pixmap *pixmap, gp_coord x, gp_coord y0,
                gp_coord y1, gp_pixel pixel)
{
	GP_CHECK_PIXMAP(pixmap);

	if (pixmap->axes_swap) {
		GP_TRANSFORM_Y_FP(pixmap, x);
		GP_TRANSFORM_X_FP(pixmap, y0);
		GP_TRANSFORM_X_FP(pixmap, y1);
		gp_hline_aa_raw(pixmap, y0, y1, x, pixel);
	} else {
		GP_TRANSFORM_X_FP(pixmap, x);
		GP_TRANSFORM_Y_FP(pixmap, y0);
		GP_TRANSFORM_Y_FP(pixmap, y1);
		gp_vline_aa_raw(pixmap, x, y0, y1, pixel);
	}
}
