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
 * Copyright (C) 2009-2013 Cyril Hrubis <metan@ucw.cz>                       *
 *                                                                           *
 *****************************************************************************/

/*

  Very basic gp_pixel tests.

 */
#include <errno.h>

#include <core/GP_Pixel.h>

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

	return TST_SUCCESS;
}

const struct tst_suite tst_suite = {
	.suite_name = "Pixel Testsuite",
	.tests = {
		{.name = "Pixel Flags",
		 .tst_fn = pixel_flags},
		{.name = NULL},
	}
};
