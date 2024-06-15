//SPDX-License-Identifier: LGPL-2.0-or-later

/*

   Copyright (c) 2014-2024 Cyril Hrubis <metan@ucw.cz>

 */

/**
 * @file gp_widget_grid.h
 * @brief A grid to position widgets.
 *
 * By far the most complex widget is the grid widget which is basically a table
 * to organize widgets.
 *
 * Grid widget distributes the space into columns and rows, i.e. all cells in a
 * given row have the same height as well as all cells in a given column have
 * the same width.  The grid can have border around it and padding between
 * cells. The border and padding consists of a constant part called padd and
 * resizable part called fill.  The constant padding is accounted for in the
 * grid widget minimal size, while the resizable fill is accounted for when
 * leftover space is being distributed.
 *
 * @note See gp_widget_hvbox.h for one dimensional variants.
 *
 * Grid JSON Example
 * -----------------
 *
 * @image html grid.png
 *
 * @include{json} demos/widgets/test_layouts/test_align.json
 *
 * @note Pass '-d layout' on application command line to highlight grid columns
 *       and rows.
 *
 * Grid JSON attributes
 * --------------------
 *
 * |  Attribute  |  Type  | Default | Description                                                    |
 * |-------------|--------|---------|----------------------------------------------------------------|
 * |  **cols**   |  uint  |   `1`   | Number of columns.                                             |
 * |  **rows**   |  uint  |   `1`   | Number of rows.                                                |
 * |  **border** | string | all `1` | See below.                                                     |
 * |  **cpad**   | string | all `1` | Horizontal border and padding size multiples.                  |
 * |  **rpad**   | string | all `1` | Vertical border and padding size multiples.                    |
 * |  **cpadf**  | string | all `0` | Horizontal border and padding fill coeficients.                |
 * |  **rpadf**  | string | all `0` | Vertical border and padding fill coeficients.                  |
 * |  **cfill**  | string | all `1` | Horizontal cell fill coeficients.                              |
 * |  **rfill**  | string | all `1` | Vertical cell fill coeficients.                                |
 * |   **pad**   |  uint  |   `1`   | Horizontal and vertical padding size multiples.                |
 * |  **frame**  |  bool  |  false  | Draws frame around grid.                                       |
 * | **uniform** |  bool  |  false  | The minimal sizes are distributed uniformly.                   |
 * | **widgets** | array  |         | Array of `cols` * `rows` widget objects.                       |
 *
 * The **pad** attribute is set before the **cpad** and **rpad**.
 *
 * Border
 * ------
 *
 * The boder can be one of `none`, `all`, `horiz`, `vert`, `top`, `bottom`,
 * `left`, `right` and can be combined with `"border": uint`.
 *
 * ### Border examples
 *
 * | JSON border                | Description                                             |
 * |----------------------------|---------------------------------------------------------|
 * | "border": "none"           | Sets all border padds to 0                              |
 * | "border": 4                | Sets all border padds to 4.                             |
 * | "border": "vert"           | Sets vertical border to 1 and clears horizontal border. |
 * | "border": top, "border": 4 | Sets top border to 4 clears rest of the borders.        |
 *
 * Padding and fill
 * ----------------
 *
 * - Numbers are divided with comma (,)
 * - Repetition can be done with number and asterisk (*)
 *
 * For example `1, 1, 1` is the same as `3 * 1`
 */

#ifndef GP_WIDGET_GRID_H
#define GP_WIDGET_GRID_H

#include <stdint.h>

/**
 * @brief A widget grid flags
 */
enum gp_widget_grid_flags {
	/** Draw a frame around the grid */
	GP_WIDGET_GRID_FRAME = 0x01,
	/** Make the cells uniform i.e. all cells have the same size */
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
 * Both the `cols` and `rows` could be 0 as the grid can be resized later.
 *
 * @param cols Number of grid columns.
 * @param rows Number of grid rows.
 * @param flags Bitwise or of the enum gp_widget_grid_flags.
 *
 * @return A widget grid.
 */
gp_widget *gp_widget_grid_new(unsigned int cols, unsigned int rows,
                              enum gp_widget_grid_flags flags);

/**
 * @brief Puts a child widget into a frame widget.
 *
 * Inserts a child widget at specified column and row into the grid and returns
 * previous child occupying the slot. Passing a NULL `child` pointer makes the
 * call equivalent to the gp_widget_grid_rem().
 *
 * @param self A grid widget.
 * @param col Grid column.
 * @param row Grid row.
 * @param child A child widget.
 *
 * @return Previous child occupying the slot or NULL if it was empty.
 */
gp_widget *gp_widget_grid_put(gp_widget *self, unsigned int col, unsigned int row,
		              gp_widget *child);

/**
 * @brief Removes child widget at col, row from a grid.
 *
 * @param self A grid widget.
 * @param col Grid column.
 * @param row Grid row.
 *
 * @return A child widget removed from the slot or NULL if it was empty.
 */
gp_widget *gp_widget_grid_rem(gp_widget *self, unsigned int col, unsigned int row);

/**
 * @brief Returns a pointer to a widget at col, row.
 *
 * @param self A grid widget.
 * @param col Grid column.
 * @param row Grid row.
 *
 * @return A child widget occupying the slot or NULL if it's empty.
 */
gp_widget *gp_widget_grid_get(gp_widget *self, unsigned int col, unsigned int row);

/**
 * @brief Deletes a child widget at col, row from a grid.
 *
 * @param self A grid widget.
 * @param col Grid column.
 * @param row Grid row.
 */
static inline void gp_widget_grid_del(gp_widget *self, unsigned col, unsigned int row)
{
	gp_widget *ret = gp_widget_grid_rem(self, col, row);

	gp_widget_free(ret);
}

/**
 * @brief Inserts new (empty) rows to the grid.
 *
 * @param self A grid widget.
 * @param row Where to insert rows.
 * @param rows How many rows to insert.
 */
void gp_widget_grid_rows_ins(gp_widget *self, unsigned int row, unsigned int rows);

/**
 * @brief Inserts new (empty) row to the grid.
 *
 * @param self A grid widget.
 * @param row Where to insert row.
 */
static inline void gp_widget_grid_row_ins(gp_widget *self, unsigned int row)
{
	gp_widget_grid_rows_ins(self, row, 1);
}

/**
 * @brief Appends a new (empty) rows at the bottom of the grid.
 *
 * @param self A grid widget.
 * @param rows How many rows to append.
 *
 * @return A row index to the first appended row.
 */
unsigned int gp_widget_grid_rows_append(gp_widget *self, unsigned int rows);

/**
 * @brief Appends a new (empty) row at the bottom of the grid.
 *
 * @param self A grid widget.
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
 * @param self A grid widget.
 * @param rows How many rows to append.
 */
void gp_widget_grid_rows_prepend(gp_widget *self, unsigned int rows);

/**
 * @brief Prepends a new (empty) row at the top of the grid.
 *
 * @param self A grid widget.
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
 * @param self A grid widget.
 * @param row Where to delete rows.
 * @param rows How may rows to delete.
 */
void gp_widget_grid_rows_del(gp_widget *self, unsigned int row, unsigned int rows);

/**
 * @brief Removes row from the grid.
 *
 * If there are any widgets in the deleted rows they are freed with
 * gp_widget_free().
 *
 * @param self A grid widget.
 * @param row Where to delete row.
 */
static inline void gp_widget_grid_row_del(gp_widget *self, unsigned int row)
{
	gp_widget_grid_rows_del(self, row, 1);
}

/**
 * @brief Inserts new (empty) columns to the grid.
 *
 * @param self A grid widget.
 * @param col Where to insert columns.
 * @param cols How many columns to insert.
 */
void gp_widget_grid_cols_ins(gp_widget *self, unsigned int col, unsigned int cols);

/**
 * @brief Inserts new (empty) column to the grid.
 *
 * @param self A grid widget.
 * @param col Where to insert column.
 */
static inline void gp_widget_grid_col_ins(gp_widget *self, unsigned int col)
{
	gp_widget_grid_cols_ins(self, col, 1);
}

/**
 * @brief Appends a new (empty) colums at the right side of the grid.
 *
 * @param self A grid widget.
 * @param cols How many columns to append.
 *
 * @return A column index to the first appended column.
 */
unsigned int gp_widget_grid_cols_append(gp_widget *self, unsigned int cols);

/**
 * @brief Appends a new (empty) colum at the right side of the grid.
 *
 * @param self A grid widget.
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
 * @param self A grid widget.
 * @param cols How many columns to append.
 */
void gp_widget_grid_cols_prepend(gp_widget *self, unsigned int cols);

/**
 * @brief Prepends a new (empty) column at the left side of the grid.
 *
 * @param self A grid widget.
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
 * @param self A grid widget.
 * @param col Where to delete columns.
 * @param cols How may columns to delete.
 */
void gp_widget_grid_cols_del(gp_widget *self, unsigned int col, unsigned int cols);

/**
 * @brief Delete column from the grid.
 *
 * If there are any widgets in the deleted rows they are freed with
 * gp_widget_free().
 *
 * @param self A grid widget.
 * @param col Where to delete column.
 */
static inline void gp_widget_grid_col_del(gp_widget *self, unsigned int col)
{
	gp_widget_grid_cols_del(self, col, 1);
}


/**
 * @brief Sets border padd or fill coeficients.
 *
 * @param self A grid widget.
 * @param border Which border(s) are set.
 * @param padd Padding coeficient no change on -1 clamped to 255.
 * @param fill A filling coeficient no change on -1 clamped to 255.
 */
void gp_widget_grid_border_set(gp_widget *self, enum gp_widget_border border,
                               int padd, int fill);

/**
 * @brief Disables grid padd and fill.
 *
 * Shortcut for gp_widget_grid_border_set(self, GP_WIDGET_BORDER_ALL, 0, 0).
 *
 * @param self A grid widget.
 */
static inline void gp_widget_grid_no_border(gp_widget *self)
{
	gp_widget_grid_border_set(self, GP_WIDGET_BORDER_ALL, 0, 0);
}

/**
 * @brief Sets grid cell column fill coeficient.
 *
 * Sets cell column fill coeficient. When fill is set to 0 the respective
 * column or row is always set to its minimal width. When greater than zero the
 * coeficient describes how aggresively is any leftover space allocated, the
 * higher the coeficient the more space is allocated. When grid is created all
 * column and row fill coeficients are set to 1.
 *
 * @param self A grid widget.
 * @param col A column.
 * @param fill A fill coeficient.
 */
void gp_widget_grid_col_fill_set(gp_widget *self, unsigned int col, uint8_t fill);

/**
 * @brief Sets grid cell row fill coeficient.
 *
 * Sets cell row fill coeficient. When fill is set to 0 the respective column
 * or row is always set to its minimal height. When greater than zero the
 * coeficient describes how aggresively is any leftover space allocated, the
 * higher the coeficient the more space is allocated. When grid is created all
 * column and row fill coeficients are set to 1.
 *
 * @param self A grid widget.
 * @param row A row.
 * @param fill A fill coeficient.
 */
void gp_widget_grid_row_fill_set(gp_widget *self, unsigned int row, uint8_t fill);

#endif /* GP_WIDGET_GRID_H */
