// SPDX-License-Identifier: GPL-2.1-or-later
/*
 * Copyright (C) 2020 Cyril Hrubis <metan@ucw.cz>
 */

#include <string.h>
#include <core/gp_seek.h>
#include "tst_test.h"

static int test_seek(enum gp_seek_whence whence, ssize_t off, size_t cur_pos,
                     size_t max_pos, int exp_ret, size_t exp_pos)
{
	int ret = gp_seek_off(off, whence, &cur_pos, max_pos);

	if (ret != exp_ret) {
		tst_msg("Wrong return for %i off=%zi cur=%zu max=%zu expected %i got %i",
			whence, off, cur_pos, max_pos, exp_ret, ret);
		return 1;
	}

	if (cur_pos != exp_pos) {
		tst_msg("Wrong offset for %i off=%zi cur=%zu max=%zu expected %zu got %zu",
			whence, off, cur_pos, max_pos, cur_pos, exp_pos);
		return 1;
	}

	return 0;
}

static int seek_cur(void)
{
	int ret = TST_PASSED;

	if (test_seek(GP_SEEK_CUR, -1, 1, 1, 0, 0))
		ret = TST_FAILED;

	if (test_seek(GP_SEEK_CUR, 10, 0, 1, 1, 0))
		ret = TST_FAILED;

	if (test_seek(GP_SEEK_CUR, 5, 0, 10, 0, 5))
		ret = TST_FAILED;

	return ret;
}

static int seek_set(void)
{
	int ret = TST_PASSED;

	if (test_seek(GP_SEEK_SET, -1, 0, 1, -1, 0))
		ret = TST_FAILED;

	if (test_seek(GP_SEEK_SET, 10, 0, 1, 1, 0))
		ret = TST_FAILED;

	if (test_seek(GP_SEEK_SET, 5, 0, 10, 0, 5))
		ret = TST_FAILED;

	return ret;
}

static int seek_end(void)
{
	int ret = TST_PASSED;

	if (test_seek(GP_SEEK_END, -2, 0, 1, -1, 0))
		ret = TST_FAILED;

	if (test_seek(GP_SEEK_END, 1, 0, 1, 1, 0))
		ret = TST_FAILED;

	if (test_seek(GP_SEEK_END, -1, 0, 6, 0, 5))
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
