//SPDX-License-Identifier: LGPL-2.0-or-later

/*

   Copyright (c) 2014-2021 Cyril Hrubis <metan@ucw.cz>

 */

#ifndef GP_WIDGET_GRID_H
#define GP_WIDGET_GRID_H

#include <stdint.h>

enum gp_widget_grid_flags {
	/** Draw a frame around the grid */
	GP_WIDGET_GRID_FRAME = 0x01,
	/** Make the cells uniform i.e. same size */
	GP_WIDGET_GRID_UNIFORM = 0x02,
};

/* row/column cell sizes, offsets and fill coefs */
struct gp_widget_grid_cell {
	unsigned int size;
	unsigned int off;
	uint8_t fill;
};

/* row/column border padding and fill coefs */
struct gp_widget_grid_border {
	uint8_t padd;
	uint8_t fill;
};

struct gp_widget_grid {
	unsigned int cols, rows;

	unsigned int focused_col;
	unsigned int focused_row;

	/* if set a widget in a grid is focused */
	int focused:1;
	/* if set frame is rendered around the grid */
	int frame:1;
	/* if set the grid all columns and all rows have the same size */
	int uniform:1;

	/* cell column sizes offsets and fill coefs */
	struct gp_widget_grid_cell *col_s;
	/* cell row sizes offsets and fill coefs */
	struct gp_widget_grid_cell *row_s;

	/* column border padding and fill coefs */
	struct gp_widget_grid_border *col_b;
	/* row border padding and fill coefs */
	struct gp_widget_grid_border *row_b;

	gp_widget **widgets;
};

/**
 * @brief Allocates and initializes a widget grid.
 *
 * @cols Number of grid columns.
 * @rows Number of grid rows.
 * @flags Bitwise or of the enum gp_widget_grid_flags.
 *
 * @return A widget grid.
 */
gp_widget *gp_widget_grid_new(unsigned int cols, unsigned int rows, int flags);

/**
 * @brief Puts a child widget into a frame widget.
 *
 * @self A grid widget.
 * @col Grid column.
 * @row Grid row.
 * @child A child widget.
 *
 * @return Previous child occupying the slot or NULL if it was empty.
 */
gp_widget *gp_widget_grid_put(gp_widget *self, unsigned int col, unsigned int row,
		              gp_widget *child);

/**
 * @brief Removes child widget at col, row from a grid.
 *
 * @self A grid widget.
 * @col Grid column.
 * @row Grid row.
 *
 * @return A child widget removed from the slot or NULL if it was empty.
 */
gp_widget *gp_widget_grid_rem(gp_widget *self, unsigned int col, unsigned int row);

/**
 * @brief Returns a pointer to a widget at col, row.
 *
 * @self A grid widget.
 * @col Grid column.
 * @row Grid row.
 *
 * @return A child widget occupying the slot or NULL if it's empty.
 */
gp_widget *gp_widget_grid_get(gp_widget *self, unsigned int col, unsigned int row);

/**
 * @brief Deletes a child widget at col, row from a grid.
 *
 * @self A grid widget.
 * @col Grid column.
 * @row Grid row.
 */
static inline void gp_widget_grid_del(gp_widget *self, unsigned col, unsigned int row)
{
	gp_widget *ret = gp_widget_grid_rem(self, col, row);

	gp_widget_free(ret);
}

/**
 * @brief Inserts new (empty) rows to the grid.
 *
 * @self A grid widget.
 * @row Where to insert rows.
 * @rows How many rows to insert.
 */
void gp_widget_grid_rows_ins(gp_widget *self, unsigned int row, unsigned int rows);

/**
 * @brief Inserts new (empty) row to the grid.
 *
 * @self A grid widget.
 * @row Where to insert row.
 */
static inline void gp_widget_grid_row_ins(gp_widget *self, unsigned int row)
{
	gp_widget_grid_rows_ins(self, row, 1);
}

/**
 * @brief Appends a new (empty) rows at the bottom of the grid.
 *
 * @self A grid widget.
 * @rows How many rows to append.
 *
 * @return A row index to the first appended row.
 */
unsigned int gp_widget_grid_rows_append(gp_widget *self, unsigned int rows);

/**
 * @brief Appends a new (empty) row at the bottom of the grid.
 *
 * @self A grid widget.
 *
 * @return A row index to the appended row.
 */
static inline unsigned int gp_widget_grid_row_append(gp_widget *self)
{
	return gp_widget_grid_rows_append(self, 1);
}

/**
 * @brief Prepends a new (empty) rows at the top of the grid.
 *
 * @self A grid widget.
 * @rows How many rows to append.
 */
void gp_widget_grid_rows_prepend(gp_widget *self, unsigned int rows);

/**
 * @brief Prepends a new (empty) row at the top of the grid.
 *
 * @self A grid widget.
 */
static inline void gp_widget_grid_row_prepend(gp_widget *self)
{
	return gp_widget_grid_rows_prepend(self, 1);
}

/**
 * @brief Delete rows from the grid.
 *
 * If there are any widgets in the deleted rows they are freed with
 * gp_widget_free().
 *
 * @self A grid widget.
 * @row Where to delete rows.
 * @rows How may rows to delete.
 */
void gp_widget_grid_rows_del(gp_widget *self, unsigned int row, unsigned int rows);

/**
 * @brief Removes row from the grid.
 *
 * If there are any widgets in the deleted rows they are freed with
 * gp_widget_free().
 *
 * @self A grid widget.
 * @row Where to delete row.
 */
static inline void gp_widget_grid_row_del(gp_widget *self, unsigned int row)
{
	gp_widget_grid_rows_del(self, row, 1);
}

/**
 * @brief Inserts new (empty) columns to the grid.
 *
 * @self A grid widget.
 * @col Where to insert columns.
 * @cols How many columns to insert.
 */
void gp_widget_grid_cols_ins(gp_widget *self, unsigned int col, unsigned int cols);

/**
 * @brief Inserts new (empty) column to the grid.
 *
 * @self A grid widget.
 * @col Where to insert column.
 */
static inline void gp_widget_grid_col_ins(gp_widget *self, unsigned int col)
{
	gp_widget_grid_cols_ins(self, col, 1);
}

/**
 * @brief Appends a new (empty) colums at the right side of the grid.
 *
 * @self A grid widget.
 * @rows How many columns to append.
 *
 * @return A column index to the first appended column.
 */
unsigned int gp_widget_grid_cols_append(gp_widget *self, unsigned int cols);

/**
 * @brief Appends a new (empty) colum at the right side of the grid.
 *
 * @self A grid widget.
 *
 * @return A column index to the appended column.
 */
static inline unsigned int gp_widget_grid_col_append(gp_widget *self)
{
	return gp_widget_grid_cols_append(self, 1);
}

/**
 * @brief Prepends a new (empty) columns at the left side of the grid.
 *
 * @self A grid widget.
 * @rows How many columns to append.
 */
void gp_widget_grid_cols_prepend(gp_widget *self, unsigned int cols);

/**
 * @brief Prepends a new (empty) column at the left side of the grid.
 *
 * @self A grid widget.
 */
static inline void gp_widget_grid_col_prepend(gp_widget *self)
{
	return gp_widget_grid_cols_prepend(self, 1);
}

/**
 * @brief Delete columns from the grid.
 *
 * If there are any widgets in the deleted columns they are freed with
 * gp_widget_free().
 *
 * @self A grid widget.
 * @col Where to delete columns.
 * @cols How may columns to delete.
 */
void gp_widget_grid_cols_del(gp_widget *self, unsigned int col, unsigned int cols);

/**
 * @brief Delete column from the grid.
 *
 * If there are any widgets in the deleted rows they are freed with
 * gp_widget_free().
 *
 * @self A grid widget.
 * @col Where to delete column.
 */
static inline void gp_widget_grid_col_del(gp_widget *self, unsigned int col)
{
	gp_widget_grid_cols_del(self, col, 1);
}

/**
 * @brief Sets both horizontal and vertical border padding and filling coeficients.
 *
 * @self A grid widget.
 * @padd Paddin coeficient.
 * @fill A filling coeficient.
 */
void gp_widget_grid_border_set(gp_widget *self, unsigned int padd, unsigned int fill);

/**
 * @brief Sets horizontal border padding and filling coeficients.
 *
 * @self A grid widget.
 * @padd Paddin coeficient.
 * @fill A filling coeficient.
 */
void gp_widget_grid_hborder_set(gp_widget *self, unsigned int padd, unsigned int fill);

/**
 * @brief Sets vertical border padding and filling coeficients.
 *
 * @self A grid widget.
 * @padd Paddin coeficient.
 * @fill A filling coeficient.
 */
void gp_widget_grid_vborder_set(gp_widget *self, unsigned int padd, unsigned int fill);

#endif /* GP_WIDGET_GRID_H */
