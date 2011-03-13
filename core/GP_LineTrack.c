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

inline void GP_LineTrackInit(struct GP_LineTrack *track, int x0, int y0,
		int x1, int y1)
{
	track->x0 = x0;
	track->y0 = y0;
	track->x1 = x1;
	track->y1 = y1;

	track->dx = abs(x1 - x0);
	track->dy = abs(y1 - y0);
	track->xstep = (x0 < x1) ? 1 : -1;
	track->ystep = (y0 < y1) ? 1 : -1;

	track->x = x0;
	track->y = y0;
	track->err = track->dx - track->dy;
	track->xmin = x0;
	track->xmax = x0;
}

inline void GP_LineTrackNext(struct GP_LineTrack *track)
{
	int x = track->x;
	int xmin = track->x;
	int xmax = track->x;

	for (;;) {
		xmin = GP_MIN(x, xmin);
		xmax = GP_MAX(x, xmax);

		if (x == track->x1 && track->y == track->y1)
			break;

		int err2 = 2*track->err;
		if (err2 > -track->dy) {
			track->err -= track->dy;
			x += track->xstep;
		}
		if (err2 < track->dx) {
			track->err += track->dx;
			track->y += track->ystep;
			break;
		}
	}

	track->x = x;
	track->xmin = xmin;
	track->xmax = xmax;
}
