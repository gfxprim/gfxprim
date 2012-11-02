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

#include "tst_log.h"
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
	
static int run_test(const struct tst_test *test, FILE *html, FILE *json)
{
	struct tst_job job;

	job.test = test;

	/* 
	 * Flush the file before forking, otherwise
	 * there would be a copy of its buffers in both
	 * child and parent and the lines in the resulting
	 * file would be repeated several times.
	 */
	fflush(html);
	fflush(json);

	tst_job_run(&job);
	tst_job_wait(&job);

	tst_log_append(&job, html, TST_LOG_HTML);
	tst_log_append(&job, json, TST_LOG_JSON);

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

	//TODO:
	FILE *html = tst_log_open(suite, "log.html", TST_LOG_HTML);
	FILE *json = tst_log_open(suite, "log.json", TST_LOG_JSON);

	for (i = 0; suite->tests[i].name != NULL; i++) {
		if (tst_name == NULL || !strcmp(tst_name, suite->tests[i].name)) {
			ret = run_test(&suite->tests[i], html, json);
			counters[ret]++;
			
			if (ret != TST_SKIPPED)
				counter++;
		}
	}

	tst_log_close(html, TST_LOG_HTML);
	tst_log_close(json, TST_LOG_JSON);

	float percents;
	
	if (counter == 0)
		percents = 100;
	else
		percents = 100.00 * counters[0] / counter;

	fprintf(stderr, "\nSummary: succedded %u out of "
	                "%u %.2f%% (skipped %u)\n",
	                counters[0], counter, percents,
		        counters[TST_SKIPPED]);
}

void tst_list_suite(const struct tst_suite *suite)
{
	int i;

	fprintf(stderr, "Testsuite: \e[1;37m%s\e[0m\n\n", suite->suite_name);

	for (i = 0; suite->tests[i].name != NULL; i++)
		fprintf(stderr, "Test: \e[1;37m%s\e[0m\n", suite->tests[i].name);
}
