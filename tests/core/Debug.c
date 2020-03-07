// SPDX-License-Identifier: GPL-2.1-or-later
/*
 * Copyright (C) 2009-2014 Cyril Hrubis <metan@ucw.cz>
 */

/*

  Test that GP_DEBUG() preserves errno.

 */
#include <errno.h>

#include <core/gp_debug.h>

#include "tst_test.h"

static int handler_called;

static void debug_handler(const struct gp_debug_msg *msg)
{
	(void)msg;
	handler_called = 1;
	errno = 0;
	tst_msg("Errno changed in debug handler");
}

/*
 * Check that GP_DEBUG() preserves errno.
 */
static int DEBUG_preserves_errno(void)
{
	gp_set_debug_handler(debug_handler);
	gp_set_debug_level(1);

	handler_called = 0;
	errno = 1;

	GP_DEBUG(1, "Debug message");

	if (!handler_called) {
		tst_msg("Debug handler wasn't called");
		return TST_FAILED;
	}

	if (errno != 1) {
		tst_msg("Errno not preserved");
		return TST_FAILED;
	}

	return TST_SUCCESS;
}

const struct tst_suite tst_suite = {
	.suite_name = "Debug",
	.tests = {
		{.name = "Debug messages preserves errno",
		 .tst_fn = DEBUG_preserves_errno},
		{.name = NULL},
	}
};
