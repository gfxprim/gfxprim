//SPDX-License-Identifier: LGPL-2.0-or-later

/*

   Copyright (c) 2014-2020 Cyril Hrubis <metan@ucw.cz>

 */

#ifndef GP_WIDGET_TABLE_H
#define GP_WIDGET_TABLE_H

enum gp_widget_table_row_op {
	GP_TABLE_ROW_RESET,
	GP_TABLE_ROW_ADVANCE,
	GP_TABLE_ROW_TELL,
};

typedef struct gp_widget_table_header {
	char *label;
	int sortable:1;
	gp_widget_tattr tattr;
	unsigned int col_min_size;
	unsigned int col_fill;
} gp_widget_table_header;

typedef struct gp_widget_table_cell {
	const char *text;
	gp_widget_tattr tattr;
} gp_widget_table_cell;

typedef struct gp_widget_table {
	unsigned int cols;
	unsigned int min_rows;

	const gp_widget_table_header *header;

	unsigned int selected_row;
	unsigned int sorted_by_col;
	int row_selected:1;
	int sorted_desc:1;

	struct timeval last_ev;

	unsigned int start_row;
	unsigned int last_rows;

	unsigned int *cols_w;

	void *priv;

	/* iterator based API */
	union {
		void *row_priv;
		unsigned long row_idx;
	};
	int (*row)(struct gp_widget *self, int op, unsigned int pos);
	gp_widget_table_cell *(*get)(struct gp_widget *self, unsigned int col);

	void (*sort)(struct gp_widget *self, unsigned int col, int desc);

	void *free;

	char buf[];
} gp_widget_table;

gp_widget *gp_widget_table_new(unsigned int cols, unsigned int min_rows,
                               const gp_widget_table_header *headers,
                               int (*row)(struct gp_widget *self,
                                          int op, unsigned int pos),
                               gp_widget_table_cell *(get)(struct gp_widget *self,
                                                           unsigned int col));

void gp_widget_table_sort_by(gp_widget *self, unsigned int col, int desc);

/*
 * Called when table content has changed and table needs to be rerendered.
 */
void gp_widget_table_refresh(gp_widget *self);

/*
 * @brief Sets first row that should be shown by the table.
 *
 * @self A table widget.
 * @off A row offset.
 */
void gp_widget_table_set_offset(gp_widget *self, unsigned int off);

#endif /* GP_WIDGET_TABLE_H */
