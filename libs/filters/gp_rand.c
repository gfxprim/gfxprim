// SPDX-License-Identifier: GPL-2.1-or-later
/*
 * Copyright (C) 2009-2012 Cyril Hrubis <metan@ucw.cz>
 */

#include <math.h>

#include "core/gp_common.h"
#include <filters/gp_rand.h>

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
