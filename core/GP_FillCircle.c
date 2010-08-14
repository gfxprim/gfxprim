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

#include <stdint.h>

GP_RetCode GP_FillCircle(GP_Context *context, int xcenter, int ycenter,
                         unsigned int r, GP_Color color)
{
	GP_CHECK_CONTEXT(context);

	GP_TRANSFORM_X(context, xcenter);
	GP_TRANSFORM_Y(context, ycenter);

	/*
	 * Draw the circle in top-down, line-per-line manner.
	 * For each line, X is calculated and a horizontal line is drawn
	 * between +X and -X, and reflected around the Y axis.
	 * X is computed in the same way as for GP_Circle().
	 */
	int x, y, error;
	for (x = 0, error = -r, y = r; y >= 0; y--) {
		while (error < 0) {
			error += 2*x + 1;
			x++;
		}
		error += -2*y + 1;

		GP_HLineInternal(context, xcenter-x+1, xcenter+x-1, ycenter-y, color);
		GP_HLineInternal(context, xcenter-x+1, xcenter+x-1, ycenter+y, color);
	}

	//TODO: See GP_Circle.c
	return GP_ESUCCESS;
}
