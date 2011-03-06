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

GP_RetCode GP_RasterizeLine(int x0, int y0, int x1, int y1, int *buf,
	int ymin, int ymax)
{
	if (buf == NULL)
		return GP_ENULLPTR;

	const int dx = abs(x1 - x0);
	const int dy = abs(y1 - y0);
	const int xstep = (x0 < x1) ? 1 : -1;
	const int ystep = (y0 < y1) ? 1 : -1;
	int err = dx - dy;
	int x = x0, y = y0;

	for (;;) {
		if (y >= ymin && y <= ymax) {
			buf[y - ymin] = x;
		}

		if (x == x1 && y == y1) break;

		int err2 = 2*err;
		if (err2 > -dy) {
			err -= dy;
			x += xstep;
		}
		if (err2 < dx) {
			err += dx;
			y += ystep;
		}
	}

	return GP_ESUCCESS;
}
