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
 * Copyright (C) 2009-2013 Cyril Hrubis <metan@ucw.cz>                       *
 *                                                                           *
 *****************************************************************************/

/*

  Test for TimeStamp monotonicity.

 */

#include <stdlib.h>
#include <unistd.h>
#include <input/GP_TimeStamp.h>

#include "tst_test.h"

#define MAX 2048

static int time_stamp_monotonicity(void)
{
	uint64_t ts = 0, nts, fts;
	int i, fail = 0;

	fts = GP_GetTimeStamp();

	for (i = 0; i < MAX; i++) {
		nts = GP_GetTimeStamp();
		if (nts < ts) {
			tst_msg("TimeStamp failed at %i %llu < %llu", i,
			        (long long unsigned)nts,
			        (long long unsigned) ts);
			fail++;
		}
		usleep(10);
	}

	tst_msg("Difference %llu", (long long unsigned)(nts - fts));

	if (fail)
		return TST_FAILED;

	return TST_SUCCESS;
}

const struct tst_suite tst_suite = {
	.suite_name = "TimeStamp Testsuite",
	.tests = {
		{.name = "TimeStamp monotonicity",
		 .tst_fn = time_stamp_monotonicity},
		{.name = NULL},
	}
};
