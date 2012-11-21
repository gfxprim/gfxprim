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

  Hilbert curve implementation.

 */

#ifndef FILTERS_GP_HILBERT_CURVE_H
#define FILTERS_GP_HILBERT_CURVE_H

struct GP_CurveState {
	/* half of the number of bits of curve size */
	unsigned int n;
	/* coordinates */
	unsigned int x, y;
	/* current curve lenght */
	unsigned int s;
};

/*
 * Resets curve to initial state i.e. x = 0, y = 0, (length) s = 0.
 */
static inline void GP_HilbertCurveInit(struct GP_CurveState *state, int n)
{
	state->n = n;
	state->s = 0;
	state->x = 0;
	state->y = 0;
}

/*
 * Variant of Lam and Shapiro
 */
static inline void GP_HilbertCurveGetXY(struct GP_CurveState *state)
{
	int sa, sb;
	/* 
	 * Older gcc thinks that x and y are used uninitialized that is not
	 * true so we silence the warning by initializing them.
	 */
	unsigned int i, temp, x = 0, y = 0;

	for (i = 0; i < 2 * state->n; i += 2) {
		sa = (state->s >> (i+1)) & 0x01;
		sb = (state->s >> i)     & 0x01;

		if ((sa ^ sb) == 0) {
			temp = x;
			x = y ^ (-sa);
			y = temp ^ (-sa);
		}
	
		x = (x >> 1) | (sa << 31);
		y = (y >> 1) | ((sa ^ sb) << 31);
	}

	state->x = x >> (32 - state->n);
	state->y = y >> (32 - state->n);
}


/*
 * Finds next X and Y
 */
static inline void GP_HilbertCurveNext(struct GP_CurveState *state)
{

	/* increment length */
	state->s++;
	/* get X and Y */
	GP_HilbertCurveGetXY(state);
}

/*
 * Returns true if we are not at curve endpoint
 */
static inline int GP_HilbertCurveContinues(struct GP_CurveState *state)
{
	return state->s < (1U<<(2*state->n));
}

#endif /* FILTERS_GP_HILBERT_CURVE_H */
