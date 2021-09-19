//SPDX-License-Identifier: LGPL-2.0-or-later

/*

   Copyright (c) 2014-2021 Cyril Hrubis <metan@ucw.cz>

 */

#include <widgets/gp_widgets.h>

#define TABLE_ROWS 4

static const char *const table_data[TABLE_ROWS][3] = {
	{"Apple", "Yes", "1"},
	{"Banana", "Yes", "2"},
	{"Orange", "Yes", "3"},
	{"Dog", "No", "4"}
};

enum tbl_ids {
	OBJECT,
	IS_FRUIT,
	NUMBER,
};

static int tbl_seek_row(gp_widget *self, int op, unsigned int pos)
{
	switch (op) {
	case GP_TABLE_ROW_RESET:
		self->tbl->row_idx = 0;
	break;
	case GP_TABLE_ROW_ADVANCE:
		self->tbl->row_idx += pos;
	break;
	case GP_TABLE_ROW_MAX:
		return TABLE_ROWS;
	}

	if (self->tbl->row_idx >= TABLE_ROWS)
		return 0;

	return 1;
}

static int tbl_get_cell(gp_widget *self, gp_widget_table_cell *cell, unsigned int col_id)
{
	unsigned int row = self->tbl->row_idx;

	switch (col_id) {
	case OBJECT:
		cell->text = table_data[row][0];
	break;
	case IS_FRUIT:
		cell->text = table_data[row][1];
	break;
	case NUMBER:
		cell->text = table_data[row][2];
	break;
	}

	return 1;
}

const gp_widget_table_col_ops table_col_ops = {
	.seek_row = tbl_seek_row,
	.get_cell = tbl_get_cell,
	.col_map = {
		{.id = "object", .idx = OBJECT},
		{.id = "is_fruit", .idx = IS_FRUIT},
		{.id = "number", .idx = NUMBER},
		{}
	}
};

int table_on_event(gp_widget_event *ev)
{
	if (ev->type != GP_WIDGET_EVENT_WIDGET)
		return 0;

	printf("Selected row %u\n", ev->self->tbl->selected_row);

	return 1;
}

int main(int argc, char *argv[])
{
	gp_widget *layout = gp_widget_layout_json("table_example.json", NULL);
	if (!layout)
		return 0;

	gp_widgets_main_loop(layout, "Table Example", NULL, argc, argv);

	return 0;
}
