// SPDX-License-Identifier: GPL-2.1-or-later
/*

   Copyright (C) 2009-2020 Cyril Hrubis <metan@ucw.cz>

 */

#include <string.h>
#include <errno.h>

#include <utils/gp_vec_str.h>

#include "tst_test.h"

static int test_printf(void)
{
	char *str;

	str = gp_vec_printf(NULL, "test %02i", 1);

	if (gp_vec_len(str) != 8) {
		tst_msg("Invalid vector len %zu expected 8", gp_vec_len(str));
		return TST_FAILED;
	}

	if (strcmp(str, "test 01")) {
		tst_msg("Invalid vector string");
		return TST_FAILED;
	}

	gp_vec_free(str);

	return TST_PASSED;
}

const struct tst_suite tst_suite = {
	.suite_name = "vector string testsuite",
	.tests = {
		{.name = "gp_vec_printf()",
		 .tst_fn = test_printf,
		 .flags = TST_CHECK_MALLOC},

		{}
	}
};
