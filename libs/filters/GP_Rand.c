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

#include <math.h>

#include "core/gp_common.h"
#include <filters/GP_Rand.h>

void gp_norm_int(int *arr, unsigned int size, int sigma, int mu)
{
	unsigned int i = 0;
	float a, b, rsq;

	GP_ASSERT(size%2 == 0);

	while (i < size) {
		/* Sample two point inside of the unit circle */
		do {
			a = (float)random() / (RAND_MAX/2) - 1;
			b = (float)random() / (RAND_MAX/2) - 1;

			rsq = a * a + b * b;

		} while (rsq >= 1 || rsq == 0);

		/* Polar form of the Box-Muller transformation */
		float mul = sqrtf(-2.0 * logf(rsq)/rsq);

		arr[i++] = mu + sigma * a * mul;
		arr[i++] = mu + sigma * b * mul;
	}
}
