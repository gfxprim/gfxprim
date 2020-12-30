//SPDX-License-Identifier: LGPL-2.0-or-later
/*

  Copyright (C) 2020 Cyril Hrubis <metan@ucw.cz>


  Implements very simple matrix abstraction on the top of a gp_vec.

  The matrix is stored in an linear array one row after another.

  The caller is supposed to maintain number of columns and rows and the library
  provides is a set of high level functions to reallocate and reshuffle the
  matrix elements.

*/

#include <utils/gp_vec.h>

#ifndef GP_MATRIX_H__
#define GP_MATRIX_H__

/*
 * @brief Allocates a matrix.
 *
 * @cols Number of columns.
 * @rows Number of rows.
 * @unit A size of a matrix element.
 *
 * @return Returns a pointer to the matrix data.
 */
static inline void *gp_matrix_new(size_t cols, size_t rows, size_t unit)
{
	return gp_vec_new(cols * rows, unit);
}

/*
 * @brief Returns an index into the vector that holds the matrix data.
 *
 * @rows Number of rows.
 * @col  Column index.
 * @row  Row index.
 *
 * @return Returns an index into a vector.
 */
static inline size_t gp_matrix_idx(size_t rows, size_t col, size_t row)
{
	return col * rows + row;
}

/*
 * @brief Inserts length columns at the offset col into the matrix.
 *
 * @self A matrix.
 * @rows Number of rows.
 * @col  Column index to insert the columns into.
 * @length How many columns we should insert.
 *
 * @return Returns a pointer to the matrix data.
 */
static inline void *gp_matrix_insert_cols(void *self, size_t rows, size_t col, size_t length)
	__attribute__((warn_unused_result));
static inline void *gp_matrix_insert_cols(void *self, size_t rows, size_t col, size_t length)
{
	return gp_vec_insert(self, col * rows, length * rows);
}

/*
 * @brief Deletes length columns at the offset col from the matrix.
 *
 * @self A matrix.
 * @rows Number of rows.
 * @col  Column index to delete the columns from.
 * @length How many columns we should delete.
 *
 * @return Returns a pointer to the matrix data.
 */
static inline void *gp_matrix_delete_cols(void *self, size_t rows, size_t col, size_t length)
	__attribute__((warn_unused_result));
static inline void *gp_matrix_delete_cols(void *self, size_t rows, size_t col, size_t length)
{
	return gp_vec_delete(self, col * rows, length * rows);
}

/*
 * @brief Inserts length rowss at the offset row into the matrix.
 *
 * @self A matrix.
 * @cols Number of columns.
 * @rows Number of rows.
 * @row  Row index to delete the rows from.
 * @length How many rows we should insert.
 *
 * @return Returns a pointer to the matrix data.
 */
void *gp_matrix_insert_rows(void *self, size_t cols, size_t rows, size_t row, size_t length)
	__attribute__((warn_unused_result));

/*
 * @brief Deletes length rows at the offset col from the matrix.
 *
 * @self A matrix.
 * @cols Number of columns.
 * @rows Number of rows.
 * @row  Row index to delete the rows from.
 * @length How many rows we should delete.
 *
 * @return Returns a pointer to the matrix data.
 */
void *gp_matrix_delete_rows(void *self, size_t cols, size_t rows, size_t row, size_t length)
	__attribute__((warn_unused_result));

/*
 * @brief Frees the matrix.
 *
 * @self A matrix.
 */
static inline void gp_matrix_free(void *self)
{
	gp_vec_free(self);
}

#endif	/* GP_MATRIX_H__ */
