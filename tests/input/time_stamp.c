// SPDX-License-Identifier: GPL-2.1-or-later
/*
 * Copyright (C) 2009-2013 Cyril Hrubis <metan@ucw.cz>
 */

/*

  Test for TimeStamp monotonicity.

 */

#include <stdlib.h>
#include <unistd.h>
#include <input/gp_time_stamp.h>

#include "tst_test.h"

#define MAX 2048

static int time_stamp_monotonicity(void)
{
	uint64_t ts = 0, nts, fts;
	int i, fail = 0;

	fts = gp_time_stamp();

	for (i = 0; i < MAX; i++) {
		nts = gp_time_stamp();
		if (nts < ts) {
			tst_msg("Time stamp failed at %i %llu < %llu", i,
			        (long long unsigned)nts,
			        (long long unsigned) ts);
			fail++;
		}
		usleep(10);
	}

	tst_msg("Difference %llu", (long long unsigned)(nts - fts));

	if (fail)
		return TST_FAILED;

	return TST_PASSED;
}

const struct tst_suite tst_suite = {
	.suite_name = "TimeStamp Testsuite",
	.tests = {
		{.name = "TimeStamp monotonicity",
		 .tst_fn = time_stamp_monotonicity},
		{.name = NULL},
	}
};
