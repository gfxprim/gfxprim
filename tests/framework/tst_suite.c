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
#include "tst_preload.h"
#include "tst_timespec.h"

#define NAME_PADD 35

int tst_suite_verbose = 0;
const char *tst_log_dir = NULL;

static void test_job_report(const struct tst_job *job)
{
	const char *name = job->test->name;
	int sec, nsec;
	const char *result = "";

	if ((job->result == TST_SUCCESS || job->result == TST_SKIPPED)
	    && !tst_suite_verbose)
		return;
	
	timespec_diff(&sec, &nsec, &job->start_time, &job->stop_time);

	switch (job->result) {
	case TST_SUCCESS:
		result = "[    \e[1;32mSUCCESS\e[0m     ]";
	break;
	case TST_SKIPPED:
		result = "[    \e[1;30mSKIPPED\e[0m     ]";
	break;
	case TST_UNTESTED:
		result = "[    \e[1;34mUNTESTED\e[0m    ]";
	break;
	case TST_INTERR:
		result = "[ \e[1;31mINTERNAL ERROR\e[0m ]";
	break;
	case TST_SIGSEGV:
		result = "[    \e[1;31mSEGFAULT\e[0m    ]";
	break;
	case TST_TIMEOUT:
		result = "[    \e[1;35mTIMEOUT\e[0m     ]";
	break;
	case TST_ABORTED:
		result = "[    \e[1;31mABORTED\e[0m     ]";
	break;
	case TST_FPE:
		result = "[ \e[1;31mFP  EXCEPTION\e[0m  ]";
	break;
	case TST_MEMLEAK:
		result = "[    \e[1;33mMEMLEAK\e[0m     ]";
	break;
	case TST_FAILED:
		result = "[    \e[1;31mFAILURE\e[0m     ]";
	break;
	case TST_MAX:
	break;
	}
		
	fprintf(stderr, "\e[1;37m%s\e[0m", name);
	
	int i;

	for (i = strlen(name); i < NAME_PADD; i++)
		fprintf(stderr, " ");

	fprintf(stderr, " finished (Time %3i.%03is)  %s\n",
	                sec, nsec/1000000, result);
	
	if (job->bench_iter) {
		for (i = 0; i < NAME_PADD; i++)
			fprintf(stderr, " ");

		fprintf(stderr, " bench CPU time %i.%06is +/- %i.%06is\n",
	               (int)job->bench_mean.tv_sec,
		       (int)job->bench_mean.tv_nsec/1000,
		       (int)job->bench_var.tv_sec,
		       (int)job->bench_var.tv_nsec/1000);
	}

	if (job->result == TST_MEMLEAK)
		tst_malloc_print(&job->malloc_stats);

	/* Now print test message store */
	tst_msg_print(&job->store);
	
	fprintf(stderr, "------------------------------------------------------"
                        "------------------------- \n");
}

static int run_test(const struct tst_test *test, FILE *json)
{
	struct tst_job job;

	job.test = test;

	/* 
	 * Flush the file before forking, otherwise
	 * there would be a copy of its buffers in both
	 * child and parent and the lines in the resulting
	 * file would be repeated several times.
	 */
	if (json)
		fflush(json);

	tst_job_run(&job);
	tst_job_wait(&job);
	
	/* report result into stdout */
	test_job_report(&job);

	if (json)
		tst_log_append(&job, json);

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

	FILE *json = NULL;
	
	if (tst_log_dir) {
		char buf[512];
		snprintf(buf, sizeof(buf), "%s/%s.json",
		         tst_log_dir, suite->suite_name);
		json = tst_log_open(suite, buf);
	}

	for (i = 0; suite->tests[i].name != NULL; i++) {
		if (tst_name == NULL || !strcmp(tst_name, suite->tests[i].name)) {
			ret = run_test(&suite->tests[i], json);
			counters[ret]++;
			
			if (ret != TST_SKIPPED)
				counter++;
		}
	}

	if (json)
		tst_log_close(json);

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
