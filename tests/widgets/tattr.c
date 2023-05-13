// SPDX-License-Identifier: GPL-2.1-or-later
/*
 * Copyright (C) 2021 Cyril Hrubis <metan@ucw.cz>
 */

#include <string.h>
#include <widgets/gp_widget_tattr.h>
#include "tst_test.h"

struct tcase {
	const char *str;
	int exp_ret;
	int enable_valign;
	gp_widget_tattr exp_attr;
};

static int test_tattr(struct tcase *a)
{
	gp_widget_tattr attr = 0;
	int flags = GP_TATTR_FONT | (a->enable_valign ? GP_TATTR_HALIGN : 0);
	int ret = gp_widget_tattr_parse(a->str, &attr, flags);

	if (ret != a->exp_ret) {
		tst_msg("Wrong return for '%s' got %i expected %i",
			a->str, ret, a->exp_ret);
		return TST_FAILED;
	}

	if (attr != a->exp_attr) {
		tst_msg("Wrong attr for '%s' got %u expected %u",
			a->str, attr, a->exp_attr);
		return TST_FAILED;
	}

	return TST_PASSED;
}

static struct tcase bold = {
	.str = "bold",
	.exp_ret = 0,
	.exp_attr = GP_TATTR_BOLD,
};

static struct tcase bold_large = {
	.str = "bold|large",
	.exp_ret = 0,
	.exp_attr = GP_TATTR_BOLD | GP_TATTR_LARGE,
};

static struct tcase bold_left = {
	.str = "bold|left",
	.exp_ret = 0,
	.enable_valign = 1,
	.exp_attr = GP_TATTR_BOLD | GP_TATTR_LEFT,
};

static struct tcase right_no_valign = {
	.str = "right",
	.exp_ret = 1,
};

static struct tcase invalid = {
	.str = "invalid",
	.exp_ret = 1,
};

static struct tcase empty = {
	.str = "",
	.exp_ret = 0,
};

static struct tcase null = {
	.str = NULL,
	.exp_ret = 0,
};

static struct tcase trailing = {
	.str = "bold|",
	.exp_ret = 1,
};

static struct tcase trailing2 = {
	.str = "bold||",
	.exp_ret = 1,
};

static struct tcase just = {
	.str = "|",
	.exp_ret = 1,
};

const struct tst_suite tst_suite = {
	.suite_name = "text attr testsuite",
	.tests = {
		{.name = "bold",
		 .tst_fn = test_tattr,
		 .data = &bold},

		{.name = "bold large",
		 .tst_fn = test_tattr,
		 .data = &bold_large},

		{.name = "bold left",
		 .tst_fn = test_tattr,
		 .data = &bold_left},

		{.name = "right no valign",
		 .tst_fn = test_tattr,
		 .data = &right_no_valign},

		{.name = "invalid",
		 .tst_fn = test_tattr,
		 .data = &invalid},

		{.name = "empty",
		 .tst_fn = test_tattr,
		 .data = &empty},

		{.name = "null",
		 .tst_fn = test_tattr,
		 .data = &null},

		{.name = "trailing |",
		 .tst_fn = test_tattr,
		 .data = &trailing},

		{.name = "trailing ||",
		 .tst_fn = test_tattr,
		 .data = &trailing2},

		{.name = "just |",
		 .tst_fn = test_tattr,
		 .data = &just},

		{.name = NULL},
	}
};
