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

#include "core/GP_FnPerBpp.h"
#include "core/GP_Transform.h"

#include "gfx/GP_HLineAA.h"
#include "gfx/GP_VLineAA.h"

void gp_hline_aa(gp_pixmap *pixmap, gp_coord x0, gp_coord x1,
                 gp_coord y, gp_pixel pixel)
{
	GP_CHECK_PIXMAP(pixmap);

	if (pixmap->axes_swap) {
		GP_TRANSFORM_Y_FP(pixmap, x0);
		GP_TRANSFORM_Y_FP(pixmap, x1);
		GP_TRANSFORM_X_FP(pixmap, y);
		gp_vline_aa_raw(pixmap, y, x0, x1, pixel);
	} else {
		GP_TRANSFORM_X_FP(pixmap, x0);
		GP_TRANSFORM_X_FP(pixmap, x1);
		GP_TRANSFORM_Y_FP(pixmap, y);
		gp_hline_aa_raw(pixmap, x0, x1, y, pixel);
	}
}
