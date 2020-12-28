// SPDX-License-Identifier: GPL-2.1-or-later
/*
 * Copyright (C) 2020 Cyril Hrubis <metan@ucw.cz>
 */

#include <string.h>
#include <core/gp_seek.h>
#include "tst_test.h"

static int test_seek(enum gp_seek_whence whence, ssize_t off,
                     size_t cur_pos, size_t max_pos, size_t exp_pos)
{
	size_t pos = gp_seek_off(off, whence, cur_pos, max_pos);

	if (pos != exp_pos) {
		tst_msg("Wrong offset for %i off=%zi cur=%zu max=%zu expected %zu got %zu",
			whence, off, cur_pos, max_pos, pos, exp_pos);
		return 1;
	}

	return 0;
}

static int seek_cur(void)
{
	int ret = TST_SUCCESS;

	if (test_seek(GP_SEEK_CUR, -1, 1, 1, 0))
		ret = TST_FAILED;

	if (test_seek(GP_SEEK_CUR, 10, 0, 1, 1))
		ret = TST_FAILED;

	if (test_seek(GP_SEEK_CUR, 5, 0, 10, 5))
		ret = TST_FAILED;

	return ret;
}

static int seek_set(void)
{
	int ret = TST_SUCCESS;

	if (test_seek(GP_SEEK_SET, -1, 0, 1, 0))
		ret = TST_FAILED;

	if (test_seek(GP_SEEK_SET, 10, 0, 1, 1))
		ret = TST_FAILED;

	if (test_seek(GP_SEEK_SET, 5, 0, 10, 5))
		ret = TST_FAILED;

	return ret;
}

static int seek_end(void)
{
	int ret = TST_SUCCESS;

	if (test_seek(GP_SEEK_END, -2, 0, 1, 0))
		ret = TST_FAILED;

	if (test_seek(GP_SEEK_END, 1, 0, 1, 1))
		ret = TST_FAILED;

	if (test_seek(GP_SEEK_END, -1, 0, 6, 5))
		ret = TST_FAILED;

	return ret;
}

const struct tst_suite tst_suite = {
	.suite_name = "seek off testsuite",
	.tests = {
		{.name = "seek cur",
		 .tst_fn = seek_cur},

		{.name = "seek set",
		 .tst_fn = seek_set},

		{.name = "seek end",
		 .tst_fn = seek_end},

		{.name = NULL},
	}
};
