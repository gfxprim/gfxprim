//SPDX-License-Identifier: LGPL-2.0-or-later
/*

  Copyright (C) 2020 Cyril Hrubis <metan@ucw.cz>

*/

#include <stdio.h>
#include <utils/gp_matrix.h>

static void print_int_matrix(int *payload, size_t cols, size_t rows)
{
	size_t col, row;
	gp_vec *vec = GP_VEC(payload);

	printf("gp_matrix { .unit = %zu, .length = %zu, .capacity = %zu, .payload = [\n ",
	       vec->unit, vec->length, vec->capacity);

	for (row = 0; row < rows; row++) {
		for (col = 0; col < cols; col++)
			printf("\t%i", payload[gp_matrix_idx(rows, col, row)]);
		printf("\n");
	}
	printf("]}\n");
}

int main(void)
{
	size_t cols = 2, rows = 3;
	int *ints = gp_matrix_new(cols, rows, sizeof(int));
	size_t col, row;

	for (row = 0; row < rows; row++) {
		for (col = 0; col < cols; col++)
			ints[gp_matrix_idx(rows, col, row)] = row + 1;
	}

	printf("Initial 3x3 matrix: ");
	print_int_matrix(ints, cols, rows);

	printf("Inserted 2 columns at 1: ");
	ints = gp_matrix_insert_cols(ints, rows, 1, 2);
	cols += 2;
	print_int_matrix(ints, cols, rows);

	printf("Deleted 1 column at 1: ");
	ints = gp_matrix_delete_cols(ints, rows, 1, 1);
	cols -= 1;
	print_int_matrix(ints, cols, rows);

	printf("Inserted 2 rows at 1: ");
	ints = gp_matrix_insert_rows(ints, cols, rows, 1, 2);
	rows += 2;
	print_int_matrix(ints, cols, rows);

	printf("Deleted 1 column at 1: ");
	ints = gp_matrix_delete_cols(ints, rows, 1, 1);
	cols -= 1;
	print_int_matrix(ints, cols, rows);

	printf("Deleted 2 rows at 1: ");
	ints = gp_matrix_delete_rows(ints, cols, rows, 1, 2);
	rows -= 2;
	print_int_matrix(ints, cols, rows);

	gp_matrix_free(ints);

	return 0;
}
