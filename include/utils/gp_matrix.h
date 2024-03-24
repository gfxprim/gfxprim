//SPDX-License-Identifier: LGPL-2.0-or-later
/*

  Copyright (C) 2020 Cyril Hrubis <metan@ucw.cz>

 */

/**
 * @file gp_matrix.h
 * @brief Two dimensional vector.
 *
 * Implements very simple matrix abstraction on the top of a gp_vec.
 *
 * The matrix is stored in an linear array one row after another.
 *
 * The caller is supposed to maintain number of columns and rows and the library
 * provides is a set of high level functions to reallocate and reshuffle the
 * matrix elements.
 */

#ifndef GP_MATRIX_H
#define GP_MATRIX_H

#include <core/gp_compiler.h>
#include <utils/gp_vec.h>

/**
 * @brief Allocates a matrix.
 *
 * @param cols Number of columns.
 * @param rows Number of rows.
 * @param unit A size of a matrix element.
 *
 * @return Returns a pointer to the matrix data.
 */
static inline void *gp_matrix_new(size_t cols, size_t rows, size_t unit)
{
	return gp_vec_new(cols * rows, unit);
}

/**
 * @brief Returns an index into the vector that holds the matrix data.
 *
 * @param rows Number of rows.
 * @param col  Column index.
 * @param row Row index.
 *
 * @return Returns an index into a vector.
 */
static inline size_t gp_matrix_idx(size_t rows, size_t col, size_t row)
{
	return col * rows + row;
}

/**
 * @brief Inserts len columns at the offset col into the matrix.
 *
 * @param self A matrix.
 * @param rows Number of rows.
 * @param col Column index to insert the columns into.
 * @param len How many columns we should insert.
 *
 * @return Returns a pointer to the matrix data.
 */
GP_WUR static inline void *gp_matrix_cols_ins(void *self, size_t rows, size_t col, size_t len)
{
	return gp_vec_ins(self, col * rows, len * rows);
}

/**
 * @brief Deletes len columns at the offset col from the matrix.
 *
 * @param self A matrix.
 * @param rows Number of rows.
 * @param col Column index to delete the columns from.
 * @param len How many columns we should delete.
 *
 * @return Returns a pointer to the matrix data.
 */
GP_WUR static inline void *gp_matrix_cols_del(void *self, size_t rows, size_t col, size_t len)
{
	return gp_vec_del(self, col * rows, len * rows);
}

/**
 * @brief Inserts len rowss at the offset row into the matrix.
 *
 * @param self A matrix.
 * @param cols Number of columns.
 * @param rows Number of rows.
 * @param row  Row index to delete the rows from.
 * @param len How many rows we should insert.
 *
 * @return Returns a pointer to the matrix data.
 */
GP_WUR void *gp_matrix_rows_ins(void *self, size_t cols, size_t rows, size_t row, size_t len);

/**
 * @brief Deletes len rows at the offset col from the matrix.
 *
 * @param self A matrix.
 * @param cols Number of columns.
 * @param rows Number of rows.
 * @param row  Row index to delete the rows from.
 * @param len How many rows we should delete.
 *
 * @return Returns a pointer to the matrix data.
 */
GP_WUR void *gp_matrix_rows_del(void *self, size_t cols, size_t rows, size_t row, size_t len);

/**
 * @brief Frees the matrix.
 *
 * @param self A matrix.
 */
static inline void gp_matrix_free(void *self)
{
	gp_vec_free(self);
}

#endif	/* GP_MATRIX_H */
