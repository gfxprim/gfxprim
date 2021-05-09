// SPDX-License-Identifier: GPL-2.1-or-later
/*
 * Copyright (C) 2021 Cyril Hrubis <metan@ucw.cz>
 */

#include <string.h>
#include <widgets/gp_widget_size_units.h>
#include "tst_test.h"

struct tcase {
	const char *str;
	int exp_ret;
	gp_widget_size exp_size;
};

static int test_size(struct tcase *a)
{
	gp_widget_size size;
	int ret = gp_widget_size_units_parse(a->str, &size);

	if (ret != a->exp_ret) {
		tst_msg("Wrong return for '%s' got %i expected %i",
			a->str, ret, a->exp_ret);
		return TST_FAILED;
	}

	if (size.px != a->exp_size.px) {
		tst_msg("Wrong px size %u expected %u",
			(unsigned int) size.px, (unsigned int) a->exp_size.px);
		return TST_FAILED;
	}

	if (size.pad != a->exp_size.pad) {
		tst_msg("Wrong pad size %u expected %u",
			(unsigned int) size.pad, (unsigned int) a->exp_size.pad);
		return TST_FAILED;
	}

	if (size.asc != a->exp_size.asc) {
		tst_msg("Wrong pad size %u expected %u",
			(unsigned int) size.asc, (unsigned int) a->exp_size.asc);
		return TST_FAILED;
	}

	return TST_SUCCESS;
}

static struct tcase px1 = {
	.str = "10",
	.exp_ret = 0,
	.exp_size = {
		.px = 10,
	}
};

static struct tcase px2 = {
	.str = "10px",
	.exp_ret = 0,
	.exp_size = {
		.px = 10,
	}
};

static struct tcase px3 = {
	.str = " 10px  ",
	.exp_ret = 0,
	.exp_size = {
		.px = 10,
	}
};

static struct tcase pad1 = {
	.str = " 33pad  ",
	.exp_ret = 0,
	.exp_size = {
		.pad = 33,
	}
};

static struct tcase pad2 = {
	.str = " 1 pad  ",
	.exp_ret = 0,
	.exp_size = {
		.pad = 1,
	}
};

static struct tcase asc = {
	.str = "1asc",
	.exp_ret = 0,
	.exp_size = {
		.asc = 1,
	}
};

static struct tcase px_asc_pad = {
	.str = "1 10asc 3pad",
	.exp_ret = 0,
	.exp_size = {
		.px = 1,
		.asc = 10,
		.pad = 3,
	}
};

static struct tcase invalid1 = {
	.str = " 10pp  ",
	.exp_ret = 1,
};

static struct tcase invalid2 = {
	.str = " xyz  ",
	.exp_ret = 1,
};

static struct tcase overflow_pad = {
	.str = "666pad",
	.exp_ret = 1,
};

static struct tcase overflow_asc = {
	.str = "666asc",
	.exp_ret = 1,
};

static struct tcase overflow_px = {
	.str = "66600",
	.exp_ret = 1,
};

static struct tcase zero_px = {
	.str = "0",
	.exp_ret = 0,
};

static struct tcase empty = {
	.str = " ",
	.exp_ret = 0,
};

const struct tst_suite tst_suite = {
	.suite_name = "widget units testsuite",
	.tests = {
		{.name = "0px",
		 .tst_fn = test_size,
		 .data = &zero_px},

		{.name = "0px 2",
		 .tst_fn = test_size,
		 .data = &empty},

		{.name = "px 1",
		 .tst_fn = test_size,
		 .data = &px1},

		{.name = "px 2",
		 .tst_fn = test_size,
		 .data = &px2},

		{.name = "px 3",
		 .tst_fn = test_size,
		 .data = &px3},

		{.name = "pad 1",
		 .tst_fn = test_size,
		 .data = &pad1},

		{.name = "pad 2",
		 .tst_fn = test_size,
		 .data = &pad2},

		{.name = "asc 1",
		 .tst_fn = test_size,
		 .data = &asc},

		{.name = "px asc pad",
		 .tst_fn = test_size,
		 .data = &px_asc_pad},

		{.name = "invalid units",
		 .tst_fn = test_size,
		 .data = &invalid1},

		{.name = "not a number",
		 .tst_fn = test_size,
		 .data = &invalid2},

		{.name = "overflow pad",
		 .tst_fn = test_size,
		 .data = &overflow_pad},

		{.name = "overflow asc",
		 .tst_fn = test_size,
		 .data = &overflow_asc},

		{.name = "overflow px",
		 .tst_fn = test_size,
		 .data = &overflow_px},

		{.name = NULL},
	}
};
