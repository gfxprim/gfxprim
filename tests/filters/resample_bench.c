// SPDX-License-Identifier: GPL-2.1-or-later
/*
 * Copyright (C) 2023-2026 Cyril Hrubis <metan@ucw.cz>
 */

#include <string.h>
#include <core/gp_core.h>
#include <filters/gp_filters.h>
#include "tst_test.h"

struct resample {
	gp_interpolation_type interp_type;
	gp_size in_w, in_h;
	gp_size out_w, out_h;
};

static int resample_bench(struct resample *res)
{
	gp_pixmap *src = gp_pixmap_alloc(res->in_w, res->in_h, GP_PIXEL_RGB888);
	gp_pixmap *dst = gp_pixmap_alloc(res->out_w, res->out_h, GP_PIXEL_RGB888);

	if (!src || !dst)
		return TST_UNTESTED;

	gp_filter_resize(src, dst, res->interp_type, NULL);

	gp_pixmap_free(src);
	gp_pixmap_free(dst);

	return TST_PASSED;
}

static struct resample resample_4024_100_nn = {
	.interp_type = GP_INTERP_NN,
	.in_w = 4024,
	.in_h = 4024,
	.out_w = 100,
	.out_h = 100,
};

static struct resample resample_1024_100_lin_lf = {
	.interp_type = GP_INTERP_LINEAR_LF_INT,
	.in_w = 1024,
	.in_h = 1024,
	.out_w = 100,
	.out_h = 100,
};

static struct resample resample_100_1024_lin = {
	.interp_type = GP_INTERP_LINEAR_INT,
	.in_w = 100,
	.in_h = 100,
	.out_w = 1024,
	.out_h = 1024,
};

const struct tst_suite tst_suite = {
	.suite_name = "Resampling testsuite",
	.tests = {
		{.name = "Resize 4024x4024 -> 100x100 Nearest neighbour",
		 .tst_fn = resample_bench,
		 .data = &resample_4024_100_nn,
		 .bench_iter = 1000},
		{.name = "Resize 1024x1024 -> 100x100 Linear LF",
		 .tst_fn = resample_bench,
		 .data = &resample_1024_100_lin_lf,
		 .bench_iter = 200},
		{.name = "Resize 1024x1024 -> 100x100 Linear",
		 .tst_fn = resample_bench,
		 .data = &resample_100_1024_lin,
		 .bench_iter = 100},
		{},
	}
};
