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

 /*

   This header contains cubic resampling function aproximation.

  */

#ifndef FILTERS_GP_CUBIC_H
#define FILTERS_GP_CUBIC_H

#define A 0.5

static inline float cubic_float(float x)
{
	if (x < 0)
		x = -x;

	if (x < 1)
		return (2 - A)*x*x*x + (A - 3)*x*x + 1;

	if (x < 2)
		return -A*x*x*x + 5*A*x*x - 8*A*x + 4*A;

	return 0;
}

#include <stdint.h>

/* Defined in GP_Cubic.gen.c */
extern int16_t GP_CubicTable[];

/*
 * Fixed point version of above.
 *
 * Both input and output value are multiplied by MUL.
 */
static inline int cubic_int(int x)
{
	if (x < 0)
		x = -x;

	if (x >= 2048)
		return 0;

	return GP_CubicTable[x];
}

#endif /* FILTERS_GP_CUBIC_H */
