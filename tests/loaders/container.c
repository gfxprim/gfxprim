// SPDX-License-Identifier: GPL-2.1-or-later
/*
 * Copyright (C) 2023 Cyril Hrubis <metan@ucw.cz>
 */

#include <errno.h>
#include <loaders/gp_loaders.h>

#include "tst_test.h"

static int regression_crash_ENOENT(void)
{
	gp_container *cont = gp_container_open("file-or-dir-does-not-exists");

	if (cont || errno != ENOENT) {
		tst_msg("Wrong return or errno!");
		return TST_FAILED;
	}

	return TST_PASSED;
}

const struct tst_suite tst_suite = {
	.suite_name = "container",
	.tests = {
		{.name = "regression crash ENOENT",
		 .tst_fn = regression_crash_ENOENT,
		 .flags = TST_MALLOC_CANARIES},

		{.name = NULL},
	}
};
