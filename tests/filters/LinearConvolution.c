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

#include <string.h>
#include <errno.h>
#include <sys/stat.h>

#include <core/gp_pixmap.h>
#include <loaders/gp_loaders.h>
#include <filters/GP_Convolution.h>

#include "tst_test.h"

static int load_resources(const char *path1, const char *path2,
                          gp_pixmap **c1, gp_pixmap **c2)
{
	*c1 = gp_load_image(path1, NULL);
	*c2 = gp_load_image(path2, NULL);

	if (*c1 == NULL || *c2 == NULL) {
		tst_err("Failed to load resource");
		return TST_UNTESTED;
	}

	return TST_SUCCESS;
}

static int test_lin_conv_box_3x3(void)
{
	gp_pixmap *in, *out;
	int ret;

	ret = load_resources("in.pgm", "out.pgm", &in, &out);

	if (ret != TST_SUCCESS)
		return ret;

	/* Apply the convolution */
	float box_3x3[] = {
		1, 1, 1,
		1, 1, 1,
		1, 1, 1,
	};

	gp_filter_kernel_2d box_3x3_kernel = {
		.w = 3,
		.h = 3,
		.div = 9,
		.kernel = box_3x3,
	};

	if (gp_filter_convolution(in, in, &box_3x3_kernel, NULL)) {
		if (errno == ENOSYS)
			return TST_SKIPPED;
	}

	/* Check result */
	//TODO

	return TST_SUCCESS;
}

static int test_h_lin_conv_box_3_raw(void)
{
	gp_pixmap *in, *out;
	int ret;

	ret = load_resources("in.pgm", "out.pgm", &in, &out);

	if (ret != TST_SUCCESS)
		return ret;

	/* Apply the convolution */
	float kernel[] = {1, 1, 1};

	if (gp_filter_hlinear_convolution_raw(in, 0, 0, in->w, in->h, in, 0, 0,
	                                kernel, 3, 3, NULL)) {
		if (errno == ENOSYS)
			return TST_SKIPPED;
	}

	/* Check result */
	//TODO

	return TST_SUCCESS;
}

static int test_v_lin_conv_box_3_raw(void)
{
	gp_pixmap *in, *out;
	int ret;

	ret = load_resources("in.pgm", "out.pgm", &in, &out);

	if (ret != TST_SUCCESS)
		return ret;

	/* Apply the convolution */
	float kernel[] = {1, 1, 1};

	if (gp_filter_vlinear_convolution_raw(in, 0, 0, in->w, in->h, in, 0, 0,
	                                kernel, 3, 3, NULL)) {
		if (errno == ENOSYS)
			return TST_SKIPPED;
	}

	/* Check result */
	//TODO

	return TST_SUCCESS;
}

const struct tst_suite tst_suite = {
	.suite_name = "Linear Convolution Testsuite",
	.tests = {
		{.name = "LinearConvolution Box 3x3",
		 .tst_fn = test_lin_conv_box_3x3,
		 .res_path = "data/conv/box_3x3/",
		 .flags = TST_TMPDIR},
		{.name = "HLinearConvolution_Raw Kern 3",
		 .tst_fn = test_h_lin_conv_box_3_raw,
		 .res_path = "data/conv/box_3x3/",
		 .flags = TST_TMPDIR},
		{.name = "VLinearConvolution_Raw Kern 3",
		 .tst_fn = test_v_lin_conv_box_3_raw,
		 .res_path = "data/conv/box_3x3/",
		 .flags = TST_TMPDIR},
		{.name = NULL}
	}
};
