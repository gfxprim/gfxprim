// SPDX-License-Identifier: GPL-2.1-or-later
/*
 * Copyright (C) 2022 Cyril Hrubis <metan@ucw.cz>
 */

#include <string.h>
#include <core/gp_core.h>
#include <text/gp_text.h>
#include "tst_test.h"

static int render_text(gp_text_style *style)
{
	unsigned int i;
	gp_pixmap *buf = gp_pixmap_alloc(1000, 1000, GP_PIXEL_RGB888);

	if (!buf)
		return TST_UNTESTED;

	for (i = 0; i < 2500; i++) {
		gp_text(buf, style, 10, 10, GP_VALIGN_BELOW | GP_ALIGN_RIGHT,
		        0xffffff, 0x000000,
			"abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890~!@#$%^&*()_+{}:\"|<>?");
	}

	gp_pixmap_free(buf);

	return TST_PASSED;
}

static int render_text_1bpp(void)
{
	return render_text(NULL);
}

static int render_text_1bpp_mul(void)
{
	gp_text_style style = {
		.font = &gp_default_font,
		.pixel_xmul = 2,
		.pixel_ymul = 2,
		.pixel_xspace = 1,
		.pixel_yspace = 1,
	};

	return render_text(&style);
}

static int render_text_8bpp(void)
{
	gp_font_face *font = gp_font_face_fc_load("arial", 10, 0);
	gp_text_style style = {
		.font = font,
	};

	if (!font)
		return TST_UNTESTED;

	int ret = render_text(&style);

	gp_font_face_free(font);

	return ret;
}

const struct tst_suite tst_suite = {
	.suite_name = "Text render benchmark",
	.tests = {
		{.name = "text 1BPP", .tst_fn = render_text_1bpp,
		 .flags = TST_TMPDIR | TST_CHECK_MALLOC,
	         .bench_iter = 25},

		{.name = "text 1BPP xmul ymul", .tst_fn = render_text_1bpp_mul,
		 .flags = TST_TMPDIR | TST_CHECK_MALLOC,
	         .bench_iter = 25},

		{.name = "text 8BPP", .tst_fn = render_text_8bpp,
		 .flags = TST_TMPDIR,
	         .bench_iter = 25},

		{.name = NULL},
	}
};
