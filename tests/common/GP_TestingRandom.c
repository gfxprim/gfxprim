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

#include <stdlib.h>

#include "GP_Common.h"
#include "GP_TestingRandom.h"

/*
 * State array for testing random generator
 *
 * The idea is to have a seperate random generator for generating
 * random inputs for tests in case tested algorithms also use
 * the random generator. Change in the tested algorithm must not 
 * change the input data generated for this or other part of the test.
 *
 * Take care when changing the values - unrelated test might start
 * exhibiting a bug, some tests may rely on the exact result.
 */
#define GP_RandomStateSize 256
static char GP_RandomTestingState[GP_RandomStateSize];
static struct random_data GP_RandomTestingData;

long int GP_TestingRandom(void)
{
	int32_t r;
	GP_ASSERT(random_r(&GP_RandomTestingData, &r) == 0);
	return r;
}

void GP_InitTestingRandom(const char *text, uint64_t seed)
{
	const char *p = text;
	for (; (p) && (*p); p++)
		seed = ((seed * 327) + *p) % 0x7B391D50A10A3LL; // TODO replace with large primes
	GP_ASSERT(initstate_r(seed, GP_RandomTestingState, GP_RandomStateSize, 
	                      &GP_RandomTestingData) == 0);
}

