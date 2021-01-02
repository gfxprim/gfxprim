//SPDX-License-Identifier: LGPL-2.0-or-later

/*

   Copyright (c) 2014-2020 Cyril Hrubis <metan@ucw.cz>

 */

#ifndef GP_WIDGET_TABLE_H__
#define GP_WIDGET_TABLE_H__

enum gp_widget_table_row_op {
	GP_TABLE_ROW_RESET,
	GP_TABLE_ROW_ADVANCE,
};

typedef struct gp_widget_table_header {
	const char *text;
	int text_align:8;
	int sortable:1;
} gp_widget_table_header;

typedef struct gp_widget_table {
	unsigned int cols;
	unsigned int min_rows;

	const gp_widget_table_header *headers;

	unsigned int *col_min_sizes;
	unsigned char *col_fills;

	unsigned int selected_row;
	unsigned int sorted_by_col;
	int row_selected:1;
	int sorted_desc:1;

	struct timeval last_ev;

	unsigned int start_row;
	unsigned int last_max_row;

	unsigned int *cols_w;

	void *priv;

	/* iterator based API */
	union {
		void *row_priv;
		unsigned long row_idx;
	};
	int (*row)(struct gp_widget *self, int op, unsigned int pos);
	const char *(*get)(struct gp_widget *self, unsigned int col);

	void (*sort)(struct gp_widget *self, unsigned int col, int desc);

	char buf[];
} gp_widget_table;

gp_widget *gp_widget_table_new(unsigned int cols, unsigned int min_rows,
                               const gp_widget_table_header *headers,
                               int (*row)(struct gp_widget *self,
                                          int op, unsigned int pos),
                               const char *(get)(struct gp_widget *self,
                                                 unsigned int col));

/*
 * Called when table content has changed and table needs to be rerendered.
 */
void gp_widget_table_refresh(gp_widget *self);

#endif /* GP_WIDGET_TABLE_H__ */
