// SPDX-License-Identifier: GPL-2.1-or-later
/*
 * Copyright (C) 2009-2014 Cyril Hrubis <metan@ucw.cz>
 */

#include <string.h>
#include <errno.h>
#include <sys/stat.h>

#include <core/gp_pixmap.h>
#include <core/gp_get_put_pixel.h>
#include <loaders/gp_loaders.h>

#include "tst_test.h"

#define READ gp_read_pnm
#include "Loader.h"
#include "PBM.h"
#include "PGM.h"
#include "PPM.h"

const struct tst_suite tst_suite = {
	.suite_name = "PNM",
	.tests = {
		/* PBM tests */
		{.name = "PBM Read 1x1 (black)",
		 .tst_fn = test_read,
		 .data = &PBM_ascii_1x1_black,
		 .flags = TST_CHECK_MALLOC},

		{.name = "PBM Read 1x1 (white)",
		 .tst_fn = test_read,
		 .data = &PBM_ascii_1x1_white,
		 .flags = TST_CHECK_MALLOC},

		{.name = "PBM Read 1x1 (black) Raw",
		 .tst_fn = test_read,
		 .data = &PBM_bin_1x1_black,
		 .flags = TST_CHECK_MALLOC},

		{.name = "PBM Read 1x1 (white) Raw",
		 .tst_fn = test_read,
		 .data = &PBM_bin_1x1_white,
		 .flags = TST_CHECK_MALLOC},

		/* PGM tests */
		 {.name = "PGM Read 1x1 1bpp (black)",
		  .tst_fn = test_read,
		  .data = &PGM_ascii_1x1_1bpp_black,
		  .flags = TST_CHECK_MALLOC},

		 {.name = "PGM Read 1x1 2bpp (black)",
		  .tst_fn = test_read,
		  .data = &PGM_ascii_1x1_2bpp_black,
		  .flags = TST_CHECK_MALLOC},

		 {.name = "PGM Read 1x1 4bpp (black)",
		  .tst_fn = test_read,
		  .data = &PGM_ascii_1x1_4bpp_black,
		  .flags = TST_CHECK_MALLOC},

		 {.name = "PGM Read 1x1 8bpp (black)",
		  .tst_fn = test_read,
		  .data = &PGM_ascii_1x1_8bpp_black,
		  .flags = TST_CHECK_MALLOC},

		 {.name = "PGM Read 1x1 1bpp (white)",
		  .tst_fn = test_read,
		  .data = &PGM_ascii_1x1_1bpp_white,
		  .flags = TST_CHECK_MALLOC},

		 {.name = "PGM Read 1x1 2bpp (white)",
		  .tst_fn = test_read,
		  .data = &PGM_ascii_1x1_2bpp_white,
		  .flags = TST_CHECK_MALLOC},

		 {.name = "PGM Read 1x1 4bpp (white)",
		  .tst_fn = test_read,
		  .data = &PGM_ascii_1x1_4bpp_white,
		  .flags = TST_CHECK_MALLOC},

		 {.name = "PGM Read 1x1 8bpp (white)",
		  .tst_fn = test_read,
		  .data = &PGM_ascii_1x1_8bpp_white,
		  .flags = TST_CHECK_MALLOC},

		 {.name = "PGM Read 1x1 1bpp (white) Raw",
		  .tst_fn = test_read,
		  .data = &PGM_bin_1x1_1bpp_white,
		  .flags = TST_CHECK_MALLOC},

		 {.name = "PGM Read 1x1 2bpp (white) Raw",
		  .tst_fn = test_read,
		  .data = &PGM_bin_1x1_2bpp_white,
		  .flags = TST_CHECK_MALLOC},

		 {.name = "PGM Read 1x1 4bpp (white) Raw",
		  .tst_fn = test_read,
		  .data = &PGM_bin_1x1_4bpp_white,
		  .flags = TST_CHECK_MALLOC},

		 {.name = "PGM Read 1x1 8bpp (white) Raw",
		  .tst_fn = test_read,
		  .data = &PGM_bin_1x1_8bpp_white,
		  .flags = TST_CHECK_MALLOC},

		/* PPM tests */
		{.name = "PPM Read 1x1 4bpp (black)",
		 .tst_fn = test_read,
		 .data = &PPM_ascii_1x1_4bpp_black,
		 .flags = TST_CHECK_MALLOC},

		{.name = "PPM Read 1x1 8bpp (black)",
		 .tst_fn = test_read,
		 .data = &PPM_ascii_1x1_8bpp_black,
		 .flags = TST_CHECK_MALLOC},

		{.name = "PPM Read 1x1 8bpp (white)",
		 .tst_fn = test_read,
		 .data = &PPM_ascii_1x1_8bpp_white,
		 .flags = TST_CHECK_MALLOC},

		{.name = "PPM Read 1x1 8bpp (white) Raw",
		 .tst_fn = test_read,
		 .data = &PPM_bin_1x1_8bpp_white,
		 .flags = TST_CHECK_MALLOC},

		{.name = NULL},
	}
};
