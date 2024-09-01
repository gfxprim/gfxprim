//SPDX-License-Identifier: LGPL-2.0-or-later

/*

   Copyright (c) 2014-2024 Cyril Hrubis <metan@ucw.cz>

 */

struct gp_widget_grid {
	unsigned int cols, rows;

	unsigned int focused_col;
	unsigned int focused_row;

	/* if set a widget in a grid is focused */
	int focused:1;

	enum gp_widget_grid_flags flags;

	/* cell column sizes offsets and fill coefs */
	struct gp_widget_grid_cell *col_s;
	/* cell row sizes offsets and fill coefs */
	struct gp_widget_grid_cell *row_s;

	/* column border padding and fill coefs */
	struct gp_widget_grid_gap *col_b;
	/* row border padding and fill coefs */
	struct gp_widget_grid_gap *row_b;

	/* Array of widgets. */
	gp_widget **widgets;
};
