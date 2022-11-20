// SPDX-License-Identifier: GPL-2.1-or-later
/*
 * Copyright (C) 2009-2022 Cyril Hrubis <metan@ucw.cz>
 */

/*

  Test for key value parser.

 */

#include <stdlib.h>
#include <unistd.h>
#include <input/gp_keys.h>

#include "tst_test.h"

struct tcase {
	const char *name;
	const int val;
};

static int test_key_val(struct tcase *t)
{
	int key_val = gp_ev_key_val(t->name);

	if (key_val != t->val) {
		tst_msg("Expected %i got %i\n", t->val, key_val);
		return TST_FAILED;
	}

	return TST_SUCCESS;
}

static struct tcase t01 = {
	.name = "KeyEnter",
	.val = GP_KEY_ENTER,
};

static struct tcase t02 = {
	.name = "KEYA",
	.val = GP_KEY_A,
};

static struct tcase t03 = {
	.name = "keySpace",
	.val = GP_KEY_SPACE,
};

static struct tcase t04 = {
	.name = "THIS_IS_NOT_KEY",
	.val = -1,
};

const struct tst_suite tst_suite = {
	.suite_name = "Key Code Testsuite",
	.tests = {
		{.name = "KeyEnter",
		 .tst_fn = test_key_val,
		 .data = &t01},

		{.name = "KEYA",
		 .tst_fn = test_key_val,
		 .data = &t02},

		{.name = "keySpace",
		 .tst_fn = test_key_val,
		 .data = &t03},

		{.name = "THIS_IS_NOT_KEY",
		 .tst_fn = test_key_val,
		 .data = &t04},

		{},
	}
};
