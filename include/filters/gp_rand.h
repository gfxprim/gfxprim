// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2009-2012 Cyril Hrubis <metan@ucw.cz>
 */

/*

  Functions to generate random numbers.

 */

#ifndef FILTERS_GP_RAND_H
#define FILTERS_GP_RAND_H

/*
 * Fills the array with size integers with Normal (Gaussian) distribution
 * defined by sigma and mu parameters.
 *
 * The size _MUST_ be even.
 */
void gp_norm_int(int *arr, unsigned int size, int sigma, int mu);

#endif /* FILTERS_GP_RAND_H */
