// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2009-2012 Cyril Hrubis <metan@ucw.cz>
 */

/*

  Hilbert curve implementation.

 */

#ifndef FILTERS_GP_HILBERT_CURVE_H
#define FILTERS_GP_HILBERT_CURVE_H

typedef struct gp_curve_state {
	/* half of the number of bits of curve size */
	unsigned int n;
	/* coordinates */
	unsigned int x, y;
	/* current curve lenght */
	unsigned int s;
} gp_curve_state;

/*
 * Resets curve to initial state i.e. x = 0, y = 0, (length) s = 0.
 */
static inline void gp_hilbert_curve_init(gp_curve_state *state, int n)
{
	state->n = n;
	state->s = 0;
	state->x = 0;
	state->y = 0;
}

/*
 * Variant of Lam and Shapiro
 */
static inline void gp_hilbert_curve_getxy(gp_curve_state *state)
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
static inline void gp_hilbert_curve_next(gp_curve_state *state)
{

	/* increment length */
	state->s++;
	/* get X and Y */
	gp_hilbert_curve_getxy(state);
}

/*
 * Returns true if we are not at curve endpoint
 */
static inline int gp_hilbert_curve_continues(gp_curve_state *state)
{
	return state->s < (1U<<(2*state->n));
}

#endif /* FILTERS_GP_HILBERT_CURVE_H */
