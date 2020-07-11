// SPDX-License-Identifier: GPL-2.1-or-later
/*

  Copyright (C) 2009-2020 Cyril Hrubis <metan@ucw.cz>

 */

#include <string.h>
#include <errno.h>

#include <utils/gp_matrix.h>

#include "tst_test.h"

static void print_int_matrix(int *payload, size_t cols, size_t rows)
{
	size_t col, row;
	gp_vec *vec = GP_VEC(payload);

	printf("gp_matrix { .unit = %zu, .length = %zu, .capacity = %zu, .payload = [\n ",
	       vec->unit, vec->length, vec->capacity);

	for (col = 0; col < cols; col++) {
		for (row = 0; row < rows; row++)
			printf("\t%i", payload[gp_matrix_idx(rows, col, row)]);
		printf("\n");
	}
	printf("]}\n");
}

static int test_matrix(void)
{
	size_t i;
	int *matrix = gp_matrix_new(10, 10, sizeof(int));

	if (!matrix) {
		tst_err("Failed to allocate matrix");
		return TST_UNTESTED;
	}

	if (gp_vec_len(matrix) != 100) {
		tst_msg("Invalid matrix lenght");
		return TST_FAILED;
	}

	for (i = 0; i < gp_vec_len(matrix); i++) {
		if (matrix[i] != 0) {
			tst_msg("Matrix not zeroed!");
			return TST_FAILED;
		}
	}

	gp_matrix_free(matrix);

	return TST_SUCCESS;
}

struct insert_test {
	size_t cols;
	size_t rows;

	int ins_cols;
	int ins_rows;

	ssize_t off;
	size_t len;

	int fail;
};

static int test_matrix_insert(struct insert_test *tst)
{
	size_t cols = tst->cols;
	size_t rows = tst->rows;
	size_t col, row;
	size_t i;
	int *matrix = gp_matrix_new(cols, rows, sizeof(int));

	if (!matrix) {
		tst_err("Failed to allocate matrix");
		return TST_UNTESTED;
	}

	i = 1;

	for (col = 0; col < cols; col++) {
		for (row = 0; row < rows; row++)
			matrix[gp_matrix_idx(rows, col, row)] = i++;
	}

	if (tst->ins_cols) {
		int *new = gp_matrix_insert_cols(matrix, rows, tst->off, tst->len);

		if (tst->fail) {
			if (new) {
				tst_msg("Insert should have failed!");
				return TST_FAILED;
			}
		} else {
			if (!new) {
				tst_msg("Insert failed");
				gp_matrix_free(matrix);
				return TST_FAILED;
			} else {
				matrix = new;
				cols += tst->len;
			}
		}
	}

	if (tst->ins_rows) {
		int *new = gp_matrix_insert_rows(matrix, cols, rows, tst->off, tst->len);

		if (tst->fail) {
			if (new) {
				tst_msg("Insert should have failed!");
				return TST_FAILED;
			}
		} else {
			if (!new) {
				tst_msg("Insert failed");
				gp_matrix_free(matrix);
				return TST_FAILED;
			} else {
				matrix = new;
				rows += tst->len;
			}
		}
	}

	if (tst->fail)
		return TST_SUCCESS;

	if (gp_vec_len(matrix) != rows * cols) {
		tst_msg("Invalid matrix lenght after insert %zu expected %zu",
			gp_vec_len(matrix), rows * cols);
		return TST_FAILED;
	}

	if (tst->ins_cols) {
		int failed = 0;
		for (col = tst->off; col < tst->len; col++) {
			for (row = 0; row < rows; row++) {
				if (matrix[gp_matrix_idx(rows, col, row)])
					failed=1;
			}
		}

		if (failed) {
			tst_msg("Inserted columns not zeroed");
			print_int_matrix(matrix, cols, rows);
			return TST_FAILED;
		}
	}

	if (tst->ins_rows) {
		int failed = 0;
		for (col = 0; col < cols; col++) {
			for (row = tst->off; row < tst->len; row++) {
				if (matrix[gp_matrix_idx(rows, col, row)])
					failed=1;
			}
		}

		if (failed) {
			tst_msg("Inserted rows not zeroed");
			print_int_matrix(matrix, cols, rows);
			return TST_FAILED;
		}
	}

	if (tst->ins_cols) {
		matrix = gp_matrix_delete_cols(matrix, rows, tst->off, tst->len);
		if (!matrix) {
			tst_msg("Delete rows failed");
			return TST_FAILED;
		}
		cols -= tst->len;
	}

	if (tst->ins_rows) {
		matrix = gp_matrix_delete_rows(matrix, cols, rows, tst->off, tst->len);
		if (!matrix) {
			tst_msg("Delete columns failed");
			return TST_FAILED;
		}
		rows -= tst->len;
	}

	if (gp_vec_len(matrix) != cols * rows) {
		tst_msg("Invalid matrix lenght after delete %zu expected %zu",
			gp_vec_len(matrix), cols * rows);
		return TST_FAILED;
	}

	for (i = 0; i < gp_vec_len(matrix); i++) {
		if (matrix[i] != (int)i+1) {
			tst_msg("Wrong data in matrix");
			print_int_matrix(matrix, cols, rows);
			return TST_FAILED;
		}
	}

	gp_matrix_free(matrix);

	return TST_SUCCESS;
}

static struct insert_test test_start = {
	.cols = 3,
	.rows = 5,
	.off = 0,
	.len = 3,
	.ins_cols = 1,
	.ins_rows = 1,
	.fail = 0,
};

static struct insert_test test_start_c = {
	.cols = 3,
	.rows = 5,
	.off = 0,
	.len = 3,
	.ins_rows = 1,
	.fail = 0,
};

static struct insert_test test_start_r = {
	.cols = 3,
	.rows = 5,
	.off = 0,
	.len = 3,
	.ins_rows = 1,
	.fail = 0,
};

static struct insert_test test_mid = {
	.cols = 3,
	.rows = 5,
	.off = 1,
	.len = 7,
	.ins_cols = 1,
	.ins_rows = 1,
	.fail = 0,
};

static struct insert_test test_mid_c = {
	.cols = 3,
	.rows = 5,
	.off = 1,
	.len = 7,
	.ins_cols = 1,
	.fail = 0,
};

static struct insert_test test_mid_r = {
	.cols = 3,
	.rows = 5,
	.off = 1,
	.len = 7,
	.ins_rows = 1,
	.fail = 0,
};

static struct insert_test test_end = {
	.cols = 3,
	.rows = 3,
	.off = 3,
	.len = 2,
	.ins_cols = 1,
	.ins_rows = 1,
	.fail = 0,
};

static struct insert_test test_end_c = {
	.cols = 3,
	.rows = 3,
	.off = 3,
	.len = 2,
	.ins_rows = 1,
	.fail = 0,
};

static struct insert_test test_end_r = {
	.cols = 3,
	.rows = 3,
	.off = 3,
	.len = 2,
	.ins_rows = 1,
	.fail = 0,
};

const struct tst_suite tst_suite = {
	.suite_name = "matrix testsuite",
	.tests = {
		{.name = "basic test",
		 .tst_fn = test_matrix,
		 .flags = TST_CHECK_MALLOC},

		{.name = "insert cols and rows at start",
		 .tst_fn = test_matrix_insert,
		 .data = &test_start,
		 .flags = TST_CHECK_MALLOC},

		{.name = "insert cols at start",
		 .tst_fn = test_matrix_insert,
		 .data = &test_start_c,
		 .flags = TST_CHECK_MALLOC},

		{.name = "insert rows at start",
		 .tst_fn = test_matrix_insert,
		 .data = &test_start_r,
		 .flags = TST_CHECK_MALLOC},

		{.name = "insert cols and rows in the middle",
		 .tst_fn = test_matrix_insert,
		 .data = &test_mid,
		 .flags = TST_CHECK_MALLOC},

		{.name = "insert cols in the middle",
		 .tst_fn = test_matrix_insert,
		 .data = &test_mid_c,
		 .flags = TST_CHECK_MALLOC},

		{.name = "insert rows in the middle",
		 .tst_fn = test_matrix_insert,
		 .data = &test_mid_r,
		 .flags = TST_CHECK_MALLOC},

		{.name = "insert cols and rows at the end",
		 .tst_fn = test_matrix_insert,
		 .data = &test_end,
		 .flags = TST_CHECK_MALLOC},

		{.name = "insert cols at the end",
		 .tst_fn = test_matrix_insert,
		 .data = &test_end_c,
		 .flags = TST_CHECK_MALLOC},

		{.name = "insert rows at the end",
		 .tst_fn = test_matrix_insert,
		 .data = &test_end_r,
		 .flags = TST_CHECK_MALLOC},


		{}
	}
};
