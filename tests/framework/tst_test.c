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

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdarg.h>

#include "tst_job.h"
#include "tst_test.h"

int tst_warn(const char *fmt, ...)
{
	va_list va;
	int ret;

	va_start(va, fmt);
	ret = vfprintf(stderr, fmt, va);
	va_end(va);

	return ret;
}
	
static int run_test(const struct tst_test *test)
{
	struct tst_job job;

	job.test = test;

	tst_job_run(&job);
	tst_job_wait(&job);

	/* Free the test message store */
	tst_msg_clear(&job.store);

	return job.result;
}

void tst_run_suite(const struct tst_suite *suite, const char *tst_name)
{
	unsigned int i;
	unsigned int counters[TST_MAX] = {};
	unsigned int counter = 0;
	int ret;

	fprintf(stderr, "Running \e[1;37m%s\e[0m\n\n", suite->suite_name);

	for (i = 0; suite->tests[i].name != NULL; i++) {
		if (tst_name == NULL || !strcmp(tst_name, suite->tests[i].name)) {
			ret = run_test(&suite->tests[i]);
			counters[ret]++;
			counter++;
		}
	}

	fprintf(stderr, "\nSummary: succedded %u out of %u (%.2f%%)\n",
	        counters[0], counter, 100.00 * counters[0] / counter);
}

void tst_list_suite(const struct tst_suite *suite)
{
	int i;

	fprintf(stderr, "Testsuite: \e[1;37m%s\e[0m\n\n", suite->suite_name);

	for (i = 0; suite->tests[i].name != NULL; i++)
		fprintf(stderr, "Test: \e[1;37m%s\e[0m\n", suite->tests[i].name);
}
