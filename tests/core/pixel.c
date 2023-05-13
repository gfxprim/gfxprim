// SPDX-License-Identifier: GPL-2.1-or-later
/*
 * Copyright (C) 2009-2013 Cyril Hrubis <metan@ucw.cz>
 */

/*

  Very basic gp_pixel tests.

 */
#include <errno.h>

#include <core/gp_pixel.h>

#include "tst_test.h"

static int pixel_flags(void)
{
	int fail = 0;

	if (!gp_pixel_has_flags(GP_PIXEL_RGB888, GP_PIXEL_IS_RGB)) {
		tst_msg("RGB888 is RGB failed");
		fail++;
	}

	if (gp_pixel_has_flags(GP_PIXEL_G1, GP_PIXEL_IS_RGB)) {
		tst_msg("G1 is RGB succeeded");
		fail++;
	}

	if (!gp_pixel_has_flags(GP_PIXEL_RGBA8888, GP_PIXEL_HAS_ALPHA)) {
		tst_msg("RGBA8888 has Alpha failed");
		fail++;
	}

	if (!gp_pixel_has_flags(GP_PIXEL_RGBA8888, GP_PIXEL_HAS_ALPHA | GP_PIXEL_IS_RGB)) {
		tst_msg("RGBA8888 has Alpha and is RGB failed");
		fail++;
	}

	if (fail)
		return TST_FAILED;

	return TST_PASSED;
}

const struct tst_suite tst_suite = {
	.suite_name = "Pixel Testsuite",
	.tests = {
		{.name = "Pixel Flags",
		 .tst_fn = pixel_flags},
		{.name = NULL},
	}
};
