//SPDX-License-Identifier: LGPL-2.0-or-later

/*

   Copyright (c) 2014-2024 Cyril Hrubis <metan@ucw.cz>

 */

/**
 * @file gp_widget_table.h
 * @brief A table widget.
 *
 * The table content is not stored in the widget, instead there are callbacks
 * that are called to get the cells content when table is being rendered.
 *
 * Table example
 * -------------
 * @include{c} demos/widgets/table_example.c
 * @include{json} demos/widgets/table_example.json
 *
 * Table JSON attributes
 * ---------------------
 *
 * |  Attribute    |  Type  | Default | Description                    |
 * |---------------|--------|---------|--------------------------------|
 * | **col_ops**   | string |         | An column ops id.              |
 * | **header**    | array  |         | Array of table header objects. |
 * | **min_rows**  |  uint  |         | Minimal number of table rows.  |
 *
 * Table Header JSON attributes
 * ----------------------------
 *
 * |  Attribute    |  Type  | Default | Description
 * |---------------|--------|---------|-----------------------------------------------------------|
 * |   **fill**    |  uint  |    0    | Column fill coeficient.                                   |
 * |    **id**     | string |         | Column id to match againts #gp_widget_table_col_desc::id. |
 * |   **label**   | string |         | If set it's the the column header label.                  |
 * |  **min_size** |  uint  |         | Minimal column width in text letters.                     |
 * |   **order**   | string |         | If column is sortable it can be sorted as asc or desc.    |
 * |   **tattr**   | string |  bold   | Column header label text attribute see #gp_widget_tattr.  |
 */

#ifndef GP_WIDGET_TABLE_H
#define GP_WIDGET_TABLE_H

#include <core/gp_compiler.h>

/** @brief Table row operation. */
enum gp_widget_table_row_op {
	/** @brief Sets current row to 0, i.e. the first row in the table. */
	GP_TABLE_ROW_RESET,
	/**
	 * @brief Moves the current row forward.
	 *
	 * Moves the current row by the op parameter elements forward.
	 *
	 * Returns non-zero if resulting row is valid and zero if not.
	 */
	GP_TABLE_ROW_ADVANCE,
	/**
	 * @brief Returns the number of table rows.
	 *
	 * Returns the number of rows in a table i.e. max_index + 1.
	 *
	 * May return -1 if the size is unknown.
	 */
	GP_TABLE_ROW_MAX
};

/**
 * @brief A table cell content.
 */
typedef struct gp_widget_table_cell {
	/** @brief An utf8 string */
	const char *text;
	/** @brief A text attribute, font and alignment. */
	gp_widget_tattr tattr;
} gp_widget_table_cell;

/**
 * @brief A table column description.
 *
 * Describes:
 * - An human readable id to index mapping, the human readable id is used by
 *   the widget JSON parser to match the column in the table header.
 * - Sortable flag, set if column could be sorted by gp_widget_table_col_ops::sort()
 */
typedef struct gp_widget_table_col_desc {
	/** @brief Column human readable ID */
	const char *id;
	/** @brief An index to map the human readable ID to */
	unsigned long idx;
	/** @brief If sort is not set this describes if column is sortable */
	int sortable:1;
} gp_widget_table_col_desc;

/**
 * @brief Table operations, defined by the application.
 *
 * This defines operations for all possible columns in the table. The table
 * header then chooses which columns to display based on this description.
 */
typedef struct gp_widget_table_col_ops {
	/**
	 * @brief Seek function for the table rows.
	 *
	 * @param self A table widget.
	 * @param op A seek operation, defines what seek should do.
	 * @param A position to seek to, is ignored for certain ops.
	 *
	 * @return See #gp_widget_table_row_op.
	 */
	int (*seek_row)(gp_widget *self, int op, unsigned int pos);
	/**
	 * @brief Returns a cell content.
	 *
	 * Retrives cells at the current row as set by the seek function.
	 *
	 * @param self A table widget.
	 * @param cell A pointer to structure to store the cell content to.
	 * @param col_idx An column index.
	 *
	 * @return TODO
	 */
	int (*get_cell)(gp_widget *self, gp_widget_table_cell *cell, unsigned int col_idx);
	/**
	 * @brief Sorts table by a column.
	 *
	 * If column is gp_widget_table_col_desc::sortable the table can be
	 * sorted by this column.
	 *
	 * @param self A table widget.
	 * @param desc If non-zero table is sorted in descending order.
	 * @param col_idx An column index for the column.
	 */
	void (*sort)(gp_widget *self, int desc, unsigned int col_idx);

	/**
	 * @brief Optional on_event handler.
	 *
	 * If set the table widget events are routed to this handler.
	 */
	int (*on_event)(gp_widget_event *ev);
	/** @brief Optional on_event handler private pointer. */
	void *on_event_priv;

	/**
	 * @brief NULL id terminated column map array.
	 *
	 * Each entry in the map describes a table column. Which columns are
	 * shown and in what order is defined in the widget table header
	 * description.
	 */
	gp_widget_table_col_desc col_map[];
} gp_widget_table_col_ops;

/**
 * @brief A table column header.
 *
 * Defines a single widget column that is shown on the screen. The table that
 * is shown in the widget is described by an array of these entries.
 */
typedef struct gp_widget_table_header {
	/**
	 * @brief Pointer to a table column descriptor.
	 *
	 * This points to a single entry in the
	 * gp_widget_table_col_ops::col_map which defines which table column is
	 * choosen for this widget column.
	 */
	gp_widget_table_col_desc *col_desc;

	/** @brief Column header label, may be NULL. */
	char *label;
	/** @brief Column header label text attributes. */
	gp_widget_tattr tattr;

	/** @brief Column minimal size in text letters. */
	unsigned int col_min_size;
	/** @brief Column fill coeficient. */
	unsigned int col_fill;
} gp_widget_table_header;

/** @brief Cached column size and minimal size, used internally by the widget. */
typedef struct gp_widget_table_col_size {
	/** @brief Current column width in pixels. */
	unsigned int size;
	/** @brief Minimal column width in pixels. */
	unsigned int min_size;
} gp_widget_table_col_size;

/**
 * @brief A structure to store data for the users of this widget.
 */
typedef struct gp_widget_table_priv {
	/**
	 * @brief A pointer to a data structure with the table data.
	 */
	void *priv;
	/**
	 * An index to store current row index for the iterator based API.
	 */
	union {
		/** @brief A current row pointer. */
		void *row_priv;
		/** @brief A current row index. */
		unsigned long row_idx;
	};
} gp_widget_table_priv;

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
	gp_coord click_y;
	int in_drag_scroll:1;
	int did_drag_scroll:1;

	unsigned int start_row;
	unsigned int last_rows;

	gp_widget_table_col_size *cols_w;

	gp_widget_table_priv priv;

	void *free;

	char buf[] GP_ALIGNED;
} gp_widget_table;

/** @brief A gp_widget_event::sub_type for a table widget. */
enum gp_widget_table_event_type {
	/** Emitted on enter or double click presseed. */
	GP_WIDGET_TABLE_TRIGGER,
	/** Emitted when table entry is selected. */
	GP_WIDGET_TABLE_SELECT,
};

/**
 * @brief Creates a new table widget.
 *
 * @param cols A number of table columns.
 * @param min_rows A minimal number of rows shown in the widget.
 * @param col_ops Columns description and callbacks to get retrieve table cells.
 * @param header Description on which rows and in which order should be shown
 *               in the widget.
 *
 * @return A newly allocated and initialid table widget.
 */
gp_widget *gp_widget_table_new(unsigned int cols, unsigned int min_rows,
                               const gp_widget_table_col_ops *col_ops,
                               const gp_widget_table_header *header);

/**
 * @brief Sorts a table widget by a column.
 *
 * This only works for a sortable columns, if column is not sortable the call
 * is no-op.
 *
 * @param self A table widget.
 * @param desc If non-zero the table is sorted in descending order.
 * @param col A column index to sort the table by.
 */
void gp_widget_table_sort_by(gp_widget *self, int desc, unsigned int col);

/**
 * @brief Request table widget refres.
 *
 * Application calls this when table content has changed and table needs to be
 * rerendered.
 *
 * @param self A table widget.
 */
void gp_widget_table_refresh(gp_widget *self);

/**
 * @brief Sets first row that should be shown by the table.
 *
 * @param self A table widget.
 * @param off A row offset.
 */
void gp_widget_table_off_set(gp_widget *self, unsigned int off);

/**
 * @brief Sets selected row.
 *
 * @param self A table widget.
 * @param row A row to be selected.
 */
void gp_widget_table_sel_set(gp_widget *self, unsigned int row);

/**
 * @brief Gets a selected row.
 *
 * Return valid result only if a row is selected, i.e.
 * gp_widget_table_sel_has() returns True.
 *
 * @param self A table widget.
 * @return A selected row index.
 */
unsigned int gp_widget_table_sel_get(gp_widget *self);

/**
 * @brief Returns if table has a fow selected.
 *
 * @param self A table widget.
 * @return True if table has a selected row.
 */
bool gp_widget_table_sel_has(gp_widget *self);

/**
 * @brief Returns a pointer to user data.
 *
 * @param self A table widget.
 * @return A pointer to user data storage for the iterator based API.
 */
gp_widget_table_priv *gp_widget_table_priv_get(gp_widget *self);

#endif /* GP_WIDGET_TABLE_H */
