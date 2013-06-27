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

/*
void GP_HLineXXYAA_Raw(GP_Context *context, GP_Coord x0, GP_Coord x1,
                       GP_Coord y, GP_Pixel pixel)
{
	GP_CHECK_CONTEXT(context);

	GP_FN_PER_BPP_CONTEXT(GP_HLine_Raw, context, context, x0, x1, y,
	                      pixel);
}
*/

void GP_HLineAA(GP_Context *context, GP_Coord x0, GP_Coord x1,
                GP_Coord y, GP_Pixel pixel)
{
	GP_CHECK_CONTEXT(context);

	if (context->axes_swap) {
		GP_TRANSFORM_Y_FP(context, x0);
		GP_TRANSFORM_Y_FP(context, x1);
		GP_TRANSFORM_X_FP(context, y);
		GP_VLineAA_Raw(context, y, x0, x1, pixel);
	} else {
		GP_TRANSFORM_X_FP(context, x0);
		GP_TRANSFORM_X_FP(context, x1);
		GP_TRANSFORM_Y_FP(context, y);
		GP_HLineAA_Raw(context, x0, x1, y, pixel);
	}
}
