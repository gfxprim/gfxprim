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

#define Ap 0.5

static inline float cubic_float(float x)
{
	if (x < 0)
		x = -x;

	if (x < 1)
		return (2 - Ap)*x*x*x + (Ap - 3)*x*x + 1;

	if (x < 2)
		return -Ap*x*x*x + 5*Ap*x*x - 8*Ap*x + 4*Ap;

	return 0;
}

#include <stdint.h>

/* Defined in gp_cubic.gen.c */
extern int16_t gp_cubic_table[];

/*
 * Fixed point version of above.
 *
 * The input and output are expected in 24.8 fixed point format
 */
static inline int cubic_int(int x)
{
	if (x < 0)
		x = -x;

	if (x >= 2048)
		return 0;

	return gp_cubic_table[x];
}

#endif /* FILTERS_GP_CUBIC_H */
