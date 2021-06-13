// SPDX-License-Identifier: GPL-2.1-or-later
/*

  Copyright (C) 2020 Richard Palethorpe <richiejp@f-m.fm>
  Copyright (C) 2009-2020 Cyril Hrubis <metan@ucw.cz>

 */

#include <string.h>
#include <errno.h>

#include <utils/gp_vec.h>

#include "tst_test.h"

static void print_int_vec(int *payload)
{
	size_t i;
	gp_vec *vec = GP_VEC(payload);

	printf("gp_vec { .unit = %zu, .capacity = %zu, .length = %zu, .payload = [\n ",
	       vec->unit, vec->capacity, vec->length);

	for (i = 0; i < vec->length; i++)
		printf("\t%zu\t= %d\n", i, payload[i]);

	if (vec->length < vec->capacity)
		printf("\t... +%zu unused capacity\n", vec->capacity - vec->length);

	printf("]}\n");
}

static int test_vec(void)
{
	size_t i;
	int *vec = gp_vec_new(10, sizeof(int));

	if (!vec) {
		tst_err("Failed to allocate vector");
		return TST_UNTESTED;
	}

	if (gp_vec_len(vec) != 10) {
		tst_msg("Invalid vector lenght");
		return TST_FAILED;
	}

	for (i = 0; i < gp_vec_len(vec); i++) {
		if (vec[i] != 0) {
			tst_msg("Vector not zeroed!");
			return TST_FAILED;
		}
	}

	gp_vec_free(vec);

	return TST_SUCCESS;
}

static int test_vec_free(void)
{
	gp_vec_free(NULL);

	return TST_SUCCESS;
}

static int test_vec_zero_size(void)
{
	int *vec = gp_vec_new(0, sizeof(int));

	if (gp_vec_len(vec) != 0) {
		tst_err("Nonzero vector size");
		return TST_FAILED;
	}

	gp_vec_free(vec);

	return TST_SUCCESS;
}

struct insert_test {
	size_t len;

	ssize_t off;
	size_t cnt;

	int fail;
};

static int test_vec_insert(struct insert_test *tst)
{
	size_t i;
	int *vec = gp_vec_new(tst->len, sizeof(int));

	if (!vec) {
		tst_err("Failed to allocate vector");
		return TST_UNTESTED;
	}

	if (gp_vec_len(vec) != tst->len) {
		tst_msg("Invalid vector lenght");
		return TST_FAILED;
	}

	for (i = 0; i < gp_vec_len(vec); i++)
		vec[i] = i;

	int *new_vec = gp_vec_ins(vec, tst->off, tst->cnt);

	if (tst->fail) {
		if (new_vec) {
			tst_msg("Insert should have failed!");
			return TST_FAILED;
		}

		gp_vec_free(vec);
		return TST_SUCCESS;
	}

	if (!new_vec) {
		tst_msg("Insert failed");
		return TST_FAILED;
	}

	vec = new_vec;

	if (gp_vec_len(vec) != tst->len + tst->cnt) {
		tst_msg("Invalid vector lenght after insert %zu expected %zu",
			gp_vec_len(vec), tst->len + tst->cnt);
		return TST_FAILED;
	}

	for (i = 0; i < tst->cnt; i++) {
		if (vec[tst->off + i]) {
			tst_msg("Inserted space not zeroed");
			print_int_vec(vec);
			return TST_FAILED;
		}
	}

	vec = gp_vec_del(vec, tst->off, tst->cnt);

	if (!vec) {
		tst_msg("Delete failed");
		return TST_FAILED;
	}

	if (gp_vec_len(vec) != tst->len) {
		tst_msg("Invalid vector lenght after delete %zu expected %zu",
			gp_vec_len(vec), tst->len);
		return TST_FAILED;
	}

	for (i = 0; i < gp_vec_len(vec); i++) {
		if (vec[i] != (int)i) {
			tst_msg("Wrong data in vector");
			print_int_vec(vec);
			return TST_FAILED;
		}
	}

	gp_vec_free(vec);

	return TST_SUCCESS;
}

static int test_vec_resize(struct insert_test *tst)
{
	char *vec = gp_vec_new(tst->len, 1);
	unsigned int i;

	vec = gp_vec_resize(vec, 10);

	if (gp_vec_len(vec) != 10) {
		tst_msg("Got wrong vector size");
		return TST_FAILED;
	}

	for (i = 0; i < 10; i++)
		vec[i] = 0xff;

	gp_vec_free(vec);

	return TST_SUCCESS;
}

static int test_vec_remove(void)
{
	int *vec = gp_vec_new(10, sizeof(int));
	int i, j;

	for (i = 0; i < 10; i++)
		vec[i] = i + 1;

	for (i = 9; i > 0; i--) {
		vec = gp_vec_remove(vec, 1);

		if (gp_vec_len(vec) != (size_t)i) {
			tst_msg("Got wrong vector size %zu", gp_vec_len(vec));
			return TST_FAILED;
		}

		for (j = 0; j < i; j++) {
			if (vec[j] != j + 1) {
				tst_msg("Got wrong vector value %i at %i", vec[j], j);
				return TST_FAILED;
			}
		}
	}

	gp_vec_free(vec);

	return TST_SUCCESS;
}

static int test_vec_dup(void)
{
	int *vec = gp_vec_new(10, sizeof(int));
	int i;

	if (!vec) {
		tst_msg("Malloc failed");
		return TST_UNTESTED;
	}

	for (i = 0; i < 10; i++)
		vec[i] = i+1;

	int *copy = gp_vec_dup(vec);
	if (!copy) {
		tst_msg("Malloc failed");
		return TST_UNTESTED;
	}

	if (gp_vec_len(copy) != 10) {
		tst_msg("Wrong vector length %zu", gp_vec_len(copy));
		return TST_FAILED;
	}

	if (gp_vec_unit(copy) != sizeof(int)) {
		tst_msg("Wrong vector unit %zu", gp_vec_unit(copy));
		return TST_FAILED;
	}

	for (i = 0; i < 10; i++) {
		if (copy[i] != i + 1) {
			tst_msg("Wrong data in vector!");
			return TST_FAILED;
		}
	}

	gp_vec_free(vec);
	gp_vec_free(copy);

	return TST_SUCCESS;
}

static struct insert_test test1 = {
	.len = 1,
	.off = 0,
	.cnt = 3,
	.fail = 0,
};

static struct insert_test test2 = {
	.len = 1,
	.off = 1,
	.cnt = 7,
	.fail = 0,
};

static struct insert_test test3 = {
	.len = 2,
	.off = 1,
	.cnt = 5,
	.fail = 0,
};

static struct insert_test test4 = {
	.len = 2,
	.off = 1,
	.cnt = 1024,
	.fail = 0,
};

static struct insert_test test_fail1 = {
	.len = 2,
	.off = 3,
	.cnt = 5,
	.fail = 1,
};

static struct insert_test resize1 = {
	.len = 1,
};

static struct insert_test resize2 = {
	.len = 10,
};

static struct insert_test resize3 = {
	.len = 100,
};

const struct tst_suite tst_suite = {
	.suite_name = "vector testsuite",
	.tests = {
		{.name = "basic test",
		 .tst_fn = test_vec,
		 .flags = TST_CHECK_MALLOC},

		{.name = "vector free on NULL",
		 .tst_fn = test_vec_free,
		 .flags = TST_CHECK_MALLOC},

		{.name = "vector dup",
		 .tst_fn = test_vec_dup,
		 .flags = TST_CHECK_MALLOC},

		{.name = "vector zero size",
		 .tst_fn = test_vec_zero_size,
		 .flags = TST_CHECK_MALLOC},

		{.name = "vector remove",
		 .tst_fn = test_vec_remove,
		 .flags = TST_CHECK_MALLOC},

		{.name = "insert test start",
		 .tst_fn = test_vec_insert,
		 .data = &test1,
		 .flags = TST_CHECK_MALLOC},

		{.name = "insert test end",
		 .tst_fn = test_vec_insert,
		 .data = &test2,
		 .flags = TST_CHECK_MALLOC},

		{.name = "insert test middle",
		 .tst_fn = test_vec_insert,
		 .data = &test3,
		 .flags = TST_CHECK_MALLOC},

		{.name = "insert test huge",
		 .tst_fn = test_vec_insert,
		 .data = &test4,
		 .flags = TST_CHECK_MALLOC},

		{.name = "insert test after end",
		 .tst_fn = test_vec_insert,
		 .data = &test_fail1,
		 .flags = TST_CHECK_MALLOC},

		{.name = "resize test 1 -> 10",
		 .tst_fn = test_vec_resize,
		 .data = &resize1,
		 .flags = TST_CHECK_MALLOC},

		{.name = "resize test 10 -> 10",
		 .tst_fn = test_vec_resize,
		 .data = &resize2,
		 .flags = TST_CHECK_MALLOC},

		{.name = "resize test 100 -> 10",
		 .tst_fn = test_vec_resize,
		 .data = &resize3,
		 .flags = TST_CHECK_MALLOC},

		{}
	}
};
