//SPDX-License-Identifier: LGPL-2.0-or-later

/*

   Copyright (c) 2014-2021 Cyril Hrubis <metan@ucw.cz>

 */

#ifndef GP_WIDGET_TABLE_H
#define GP_WIDGET_TABLE_H

enum gp_widget_table_row_op {
	/** Sets current row to 0 */
	GP_TABLE_ROW_RESET,
	/** Moves forward by pos parameter */
	GP_TABLE_ROW_ADVANCE,
	/** Returns number of rows or -1 if not implemented */
	GP_TABLE_ROW_MAX
};

typedef struct gp_widget_table_cell {
	const char *text;
	gp_widget_tattr tattr;
} gp_widget_table_cell;

/**
 * A table column description.
 *
 * Describes:
 * - an human readable id to index mapping
 * - sortable flag, set if column could be sorted by ops->sort()
 */
typedef struct gp_widget_table_col_dsc {
	/** Column human readable ID */
	const char *id;
	/** An index to map the human readable ID to */
	unsigned long idx;
	/** If sort is not set this describes if column is sortable */
	int sortable:1;
} gp_widget_table_col_dsc;

/**
 * Table operations, defined by the application.
 *
 * This defines operations for all possible columns in the table. The table
 * header then chooses which columns to display based on this description.
 */
typedef struct gp_widget_table_col_ops {
	int (*seek_row)(gp_widget *self, int op, unsigned int pos);
	int (*get_cell)(gp_widget *self, gp_widget_table_cell *cell, unsigned int col_idx);
	void (*sort)(gp_widget *self, int desc, unsigned int col_idx);

	/** Optional on_event handler */
	int (*on_event)(gp_widget_event *ev);
	void *on_event_priv;

	/** NULL id terminated column map array */
	gp_widget_table_col_dsc col_map[];
} gp_widget_table_col_ops;

/**
 * Table header, defines which coluns are shown to the user.
 */
typedef struct gp_widget_table_header {
	/** Pointer to a table column descriptor */
	gp_widget_table_col_dsc *col_dsc;

	/** Column label, may be NULL */
	char *label;

	/** Label text attributes */
	gp_widget_tattr tattr;

	/** Column size and fill */
	unsigned int col_min_size;
	unsigned int col_fill;
} gp_widget_table_header;

/**
 * Column size and minimal size in pixels
 */
typedef struct gp_widget_table_col {
	unsigned int size;
	unsigned int min_size;
} gp_widget_table_col;

typedef struct gp_widget_table {
	unsigned int cols;
	unsigned int min_rows;

	/** Header defines columns show to the user */
	const gp_widget_table_header *header;
	/** Defines all possible columns */
	gp_widget_table_col_ops col_ops;

	unsigned int selected_row;
	unsigned int sorted_by_col;
	int row_selected:1;
	int sorted_desc:1;
	int needs_header:1;

	uint64_t last_click;

	unsigned int start_row;
	unsigned int last_rows;

	gp_widget_table_col *cols_w;

	void *priv;

	/* iterator based API */
	union {
		void *row_priv;
		unsigned long row_idx;
	};

	void *free;

	char buf[];
} gp_widget_table;

/**
 * @brief Event sub_type for table widget events.
 */
enum gp_widget_table_event_type {
	/** Emitted on enter or double click presseed */
	GP_WIDGET_TABLE_TRIGGER,
	/** Emitted when table entry is selected */
	GP_WIDGET_TABLE_SELECT,
};

gp_widget *gp_widget_table_new(unsigned int cols, unsigned int min_rows,
                               const gp_widget_table_col_ops *col_ops,
                               const gp_widget_table_header *header);

void gp_widget_table_sort_by(gp_widget *self, int desc, unsigned int col);

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
void gp_widget_table_off_set(gp_widget *self, unsigned int off);

/*
 * @brief Sets selected row.
 *
 * @self A table widget.
 * @off A row.
 */
void gp_widget_table_sel_set(gp_widget *self, unsigned int row);

#endif /* GP_WIDGET_TABLE_H */
