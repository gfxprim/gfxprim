//SPDX-License-Identifier: LGPL-2.0-or-later

/*

   Copyright (c) 2014-2020 Cyril Hrubis <metan@ucw.cz>

 */

#ifndef GP_WIDGET_GRID_H__
#define GP_WIDGET_GRID_H__

#include <stdint.h>

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

	/** column/row sizes */
	unsigned int *cols_w;
	unsigned int *rows_h;

	/** column/row offsets */
	unsigned int *cols_off;
	unsigned int *rows_off;

	/* padding and padding fills */
	uint8_t *col_padds;
	uint8_t *row_padds;
	uint8_t *col_pfills;
	uint8_t *row_pfills;

	/* cell fill coeficients */
	uint8_t *col_fills;
	uint8_t *row_fills;

	gp_widget **widgets;
};

/**
 * @brief Allocates and initializes a widget grid.
 *
 * @cols Number of grid columns.
 * @rows Number of grid rows.
 *
 * @return A widget grid.
 */
gp_widget *gp_widget_grid_new(unsigned int cols, unsigned int rows);

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
 * @brief Add a new (empty) rows at the end of the grid.
 *
 * @self A grid widget.
 * @rows How many rows to add.
 */
void gp_widget_grid_add_rows(gp_widget *self, unsigned int rows);

/**
 * @brief Inserts new (empty) rows to the grid.
 *
 * @self A grid widget.
 * @row Where to insert rows.
 * @rows How many rows to insert.
 */
void gp_widget_grid_insert_rows(gp_widget *self, unsigned int row, unsigned int rows);

/**
 * @brief Add a new (empty) row at the end of the grid.
 *
 * @self A grid widget.
 */
static inline void gp_widget_grid_add_row(gp_widget *self)
{
	gp_widget_grid_add_rows(self, 1);
}

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

#endif /* GP_WIDGET_GRID_H__ */
