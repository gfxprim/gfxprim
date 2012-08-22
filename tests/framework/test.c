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

#include "tst_test.h"
#include "tst_alloc_barriers.h"

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
	void *p;

	p = malloc(4);
	p = malloc(3);

	free(p);

	tst_report(0, "Leaking 1 chunks 4 bytes total");

	return TST_SUCCESS;
}

int malloc_ok_fn(void)
{
	void *p;

	p = malloc(100);
	free(p);

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

const struct tst_suite suite = {
	.suite_name = "Testing Framework Example",
	.tests = {
		{.name = "Success test", .tst_fn = success_fn},
		{.name = "Sigsegv test", .tst_fn = sigsegv_fn},
		{.name = "Failed test", .tst_fn = failed_fn},
		{.name = "Stack overflow test", .tst_fn = stack_overflow_fn},
		{.name = "Timeout test", .tst_fn = timeout_fn, .timeout = 1},
		{.name = "Tempdir test", .tst_fn = temp_dir_fn, .flags = TST_TMPDIR},
		{.name = "Mem Leak test", .tst_fn = malloc_leak_fn, .flags = TST_MALLOC_CHECK},
		{.name = "Mem Ok test", .tst_fn = malloc_ok_fn, .flags = TST_MALLOC_CHECK},
		{.name = "Double free()", .tst_fn = double_free},
		{.name = "Barrier allocation", .tst_fn = barrier_allocation},
		{.name = NULL},
	}
};

int main(void)
{
	fprintf(stderr, "(Listing testsuite tests)\n");
	tst_list_suite(&suite);
	
	fprintf(stderr, "\n(Running selected test)\n");
	tst_run_suite(&suite, "Sigsegv test");

	fprintf(stderr, "\n(Running whole suite)\n");
	tst_run_suite(&suite, NULL);

	return 0;
}
