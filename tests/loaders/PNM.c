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
 * Copyright (C) 2009-2014 Cyril Hrubis <metan@ucw.cz>                       *
 *                                                                           *
 *****************************************************************************/

#include <string.h>
#include <errno.h>
#include <sys/stat.h>

#include <core/GP_Pixmap.h>
#include <core/GP_GetPutPixel.h>
#include <loaders/GP_Loaders.h>

#include "tst_test.h"

#define READ GP_ReadPNM
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
