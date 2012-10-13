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

#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>

#include "tst_test.h"
#include "tst_alloc_barriers.h"
#include "tst_preload_FILE.h"

int success_fn(void)
{
	tst_report(0, "This test does nothing");
	tst_report(0, "But successfully");
	
	return TST_SUCCESS;
}

int sigsegv_fn(void)
{
	return *(int*)NULL;
}

int failed_fn(void)
{
	return TST_FAILED;
}

int stack_overflow_fn(void)
{
	int ret = stack_overflow_fn() + stack_overflow_fn();

	return ret;
}

int timeout_fn(void)
{
	tst_report(0, "Sleeping for ten seconds");
	sleep(10);
	return TST_SUCCESS;
}

int temp_dir_fn(void)
{
	char buf[256], *res;

	/* log current working directory */
	res = getcwd(buf, sizeof(buf));
	tst_report(0, "CWD is '%s'", res);

	return TST_SUCCESS;
}

int malloc_leak_fn(void)
{
	void *p, *q, *r;
	
	q = malloc(100);
	p = malloc(4);
	p = malloc(3);
	r = malloc(20);

	free(p);
	free(q);
	free(r);

	tst_report(0, "Leaking 1 chunks 4 bytes total");

	return TST_SUCCESS;
}

int malloc_ok_fn(void)
{
	unsigned int perm[20] = {
		1,  3,  2,  6,  4,  5,  0,  9,  8, 14,
		7, 11, 13, 10, 12, 19, 17, 16, 15, 18,
	};

	unsigned int i;
	void *p[20];

	for (i = 0; i < 20; i++)
		p[i] = malloc((7 * i) % 127);

	for (i = 0; i < 20; i++)
		free(p[perm[i]]);

	return TST_SUCCESS;
}

int double_free(void)
{
	void *p = malloc(100);

	free(p);
	free(p);

	return TST_SUCCESS;
}

int barrier_allocation(void)
{
	char *buf = tst_alloc_barrier_right(31);

	int i;

	for (i = 0; i < 31; i++)
		buf[i] = 0;

	tst_report(0, "About to use address after the buffer with barrier");
	
	buf[31] = 0;

	tst_report(0, "This is not printed at all");

	return TST_SUCCESS;
}

int fail_FILE(void)
{
	struct tst_fail_FILE failures[] = {
		{.path = "test_fail_fopen", .call = TST_FAIL_FOPEN, .err = EPERM},
		{.path = "test_fail_fclose", .call = TST_FAIL_FCLOSE, .err = ENOSPC},
		{.path = NULL}
	};

	tst_fail_FILE_register(failures);

	int fail = 0;
	FILE *f;
	
	f = fopen("test_fail_fclose", "w");

	if (f == NULL) {
		tst_report(0, "Failed to open 'test_fail_fclose' for writing: %s",
		           strerror(errno));
		fail = 1;
	}

	tst_report(0, "Correctly opened 'test_fail_fclose'");

	int ret = fclose(f);

	if (ret == 0 || errno != ENOSPC) {
		tst_report(0, "Failed to fail to close 'test_fail_fclose'");
		fail = 1;
	}

	tst_report(0, "Correctly failed to close 'test_fail_fclose'");

	f = fopen("test_fail_fopen", "w");

	if (f != NULL && errno != EPERM) {
		tst_report(0, "Failed to fail to open 'test_fail_fopen'");
		fclose(f);
		fail = 1;
	}

	tst_report(0, "Correctly failed to open 'test_fail_fopen'"); 

	if (fail)
		return TST_FAILED;
	
	return TST_SUCCESS;
}

static int skipped_fn(void)
{
	return TST_SKIPPED;
}

static int res_fn(void)
{
	if (access("test.c", R_OK) == 0) 
		tst_report(0, "File correctly copied");

	return TST_SUCCESS;
}

static int fpe_fn(void)
{
	/* its volatile so compiler doesn't detect the division by zero */
	volatile int i = 0;

	return 1/i;
}

/*
 * Let's benchmark memset.
 */ 
static int benchmark_fn(void)
{
	char buf[256];
	unsigned int i;

	for (i = 0; i < 4000000; i++)
		memset(buf, i%100, sizeof(buf));

	return TST_SUCCESS;
}

const struct tst_suite tst_suite = {
	.suite_name = "Testing Framework Example",
	.tests = {
		{.name = "Success test", .tst_fn = success_fn},
		{.name = "Skipped test", .tst_fn = skipped_fn},
		{.name = "Sigsegv test", .tst_fn = sigsegv_fn},
		{.name = "Failed test", .tst_fn = failed_fn},
		{.name = "Stack overflow test", .tst_fn = stack_overflow_fn},
		{.name = "Timeout test", .tst_fn = timeout_fn, .timeout = 1},
		{.name = "Tempdir test", .tst_fn = temp_dir_fn, .flags = TST_TMPDIR},
		{.name = "Mem Leak test", .tst_fn = malloc_leak_fn, .flags = TST_CHECK_MALLOC},
		{.name = "Mem Ok test", .tst_fn = malloc_ok_fn, .flags = TST_CHECK_MALLOC},
		{.name = "Double free()", .tst_fn = double_free},
		{.name = "Barrier allocation", .tst_fn = barrier_allocation},
		{.name = "Failed FILE", .tst_fn = fail_FILE, .flags = TST_TMPDIR},
		{.name = "Resource", .tst_fn = res_fn, .flags = TST_TMPDIR,
		 .res_path = "test.c"},
		{.name = "FP exception", .tst_fn = fpe_fn},
		{.name = "Benchmark test", .tst_fn = benchmark_fn, .bench_iter = 10},
		{.name = NULL},
	}
};
