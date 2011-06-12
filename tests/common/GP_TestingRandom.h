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
 *                                                                           *
 *****************************************************************************/

/*
 * These routines use a pseudorandom generator with internal state.
 * Use this only to provide tests with pseudo-random inputs.
 * The seed is initialized with the test name, so the random numbers
 * (and therefore inputs) are the same for a test, and do not depend
 * on random() called in the tested routines or other tests.
 */

#ifndef GP_TESTING_RANDOM_H
#define GP_TESTING_RANDOM_H

#include <stdint.h>

/* 
 * Return next random value from the testing random generator
 */
long int GP_TestingRandom(void);

/*
 * Initialize the random generator to a seed computed from the given string
 * and the given seed value. Needs to be called at least once. 
 *
 * The string may be for example the name of the test, the seed the number of test iteration.
 */
void GP_InitTestingRandom(const char *text, uint64_t seed);

#endif /* GP_TESTING_RANDOM_H */
