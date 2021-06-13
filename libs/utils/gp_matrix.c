//SPDX-License-Identifier: LGPL-2.0-or-later
/*

  Copyright (C) 2020-2021 Cyril Hrubis <metan@ucw.cz>

*/

#include <string.h>
#include <core/gp_common.h>
#include <core/gp_debug.h>
#include <utils/gp_vec.h>
#include <utils/gp_matrix.h>

void *gp_matrix_rows_ins(void *self, size_t cols, size_t rows, size_t row, size_t length)
{
	gp_vec *vec = GP_VEC(self);
	size_t col;

	if (row > rows) {
		GP_WARN("Row (%zu) out of matrix %p rows %zu",
			row, self, rows);
		return NULL;
	}

	vec = gp_vec_expand_(vec, length * cols);
	if (!vec)
		return NULL;

	for (col = cols; col > 0; col--) {
		size_t old_col = (col-1) * rows;
		size_t new_col = (col-1) * (rows + length);

		memmove(vec->payload + (new_col + row + length) * vec->unit,
		        vec->payload + (old_col + row) * vec->unit, (rows - row) * vec->unit);
		memset(vec->payload + (new_col + row) * vec->unit, 0, length * vec->unit);
		memmove(vec->payload + new_col * vec->unit,
			vec->payload + old_col * vec->unit, row * vec->unit);
	}

	return (void*)vec->payload;
}


void *gp_matrix_rows_del(void *self, size_t cols, size_t rows, size_t row, size_t length)
{
	gp_vec *vec = GP_VEC(self);
	size_t col;

	if (row + length > rows) {
		GP_WARN("Block (%zu-%zu) out of matrix %p rows %zu",
			row, length, self, rows);
		return NULL;
	}

	for (col = 0; col < cols; col++) {
		size_t old_col = col * rows;
		size_t new_col = col * (rows - length);

		memmove(vec->payload + new_col * vec->unit,
		        vec->payload + old_col * vec->unit,
			row * vec->unit);

		memmove(vec->payload + (new_col + row) * vec->unit,
		        vec->payload + (old_col + row + length) * vec->unit,
			(rows - row - length) * vec->unit);
	}

	vec = gp_vec_shrink_(vec, length * cols);

	return (void*)vec->payload;
}
