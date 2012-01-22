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
 * Copyright (C) 2011      Tomas Gavenciak <gavento@ucw.cz>                  *
 * Copyright (C) 2012      Cyril Hrubis <metan@ucw.cz>                       *
 * Copyright (C) 2012      Jiri Dluhos <jiri.bluebear.dluhos@gmail.com>      *
 *                                                                           *
 *****************************************************************************/

#include "GP_AngleUtils.h"

#include <math.h>

double GP_NormalizeAngle(double phi)
{
	// clamp angle to <-2*pi, 2*pi>
	double phi2 = fmod(phi, 2*M_PI);

	// clamp angle to <0, 2*pi>
	if (phi2 < 0)
		phi2 += 2*M_PI;

	return phi2;
}

int GP_AngleInRange(double angle, double start, double end)
{
	if (start < end) {
		return (angle >= start && angle <= end);
	} else {
		return (angle >= start && angle <= 2*M_PI)
			|| (angle >= 0 && angle <= end);
	}
}