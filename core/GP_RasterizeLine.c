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
	int ymin, int ymax, int flags)
{
	if (buf == NULL)
		return GP_ENULLPTR;

	const int dx = abs(x1 - x0);
	const int dy = abs(y1 - y0);
	const int xstep = (x0 < x1) ? 1 : -1;
	const int ystep = (y0 < y1) ? 1 : -1;
	int err = dx - dy;
	int x = x0, y = y0;
	int touched = 0;

	for (;;) {
		if (y >= ymin && y <= ymax) {
			if (touched) {
				if (flags & GP_KEEP_XMIN)
					buf[y - ymin] = GP_MIN(x, buf[y - ymin]);
				else if (flags & GP_KEEP_XMAX)
					buf[y - ymin] = GP_MAX(x, buf[y - ymin]);
				else
					buf[y - ymin] = x;
			} else {
				buf[y - ymin] = x;
				touched = 1;
			}
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
			touched = 0;
		}
	}

	return GP_ESUCCESS;
}
