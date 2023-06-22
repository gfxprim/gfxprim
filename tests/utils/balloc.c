// SPDX-License-Identifier: GPL-2.1-or-later
/*

  Copyright (C) 2023 Cyril Hrubis <metan@ucw.cz>

 */

#include <utils/gp_block_alloc.h>

#include "tst_test.h"

int balloc_arrays_test(void)
{
	gp_balloc_pool *pool = NULL;
	int i;

	int *arr1 = gp_balloc(&pool, 20 * sizeof(int));
	int *arr2 = gp_balloc(&pool, 10 * sizeof(int));

	for (i = 0; i < 20; i++)
		arr1[i] = i;

	for (i = 0; i < 10; i++)
		arr2[i] = i;

	for (i = 0; i < 20; i++) {
		if (arr1[i] != i) {
			return TST_FAILED;
			tst_msg("Wrong value in array!");
		}
	}

	for (i = 0; i < 10; i++) {
		if (arr2[i] != i) {
			return TST_FAILED;
			tst_msg("Wrong value in array!");
		}
	}

	gp_bfree(&pool);

	tst_msg("We haven't crashed");
	return TST_PASSED;
}

const struct tst_suite tst_suite = {
	.suite_name = "balloc testsuite",
	.tests = {
		{.name = "balloc arrays test",
		 .tst_fn = balloc_arrays_test},

		{}
	}
};
