//SPDX-License-Identifier: LGPL-2.0-or-later

/*

   Copyright (c) 2021-2023 Cyril Hrubis <metan@ucw.cz>

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
	gp_widget_table_priv *tbl_priv = gp_widget_table_priv_get(self);

	switch (op) {
	case GP_TABLE_ROW_RESET:
		tbl_priv->row_idx = 0;
	break;
	case GP_TABLE_ROW_ADVANCE:
		tbl_priv->row_idx += pos;
	break;
	case GP_TABLE_ROW_MAX:
		return TABLE_ROWS;
	}

	if (tbl_priv->row_idx >= TABLE_ROWS)
		return 0;

	return 1;
}

static int tbl_get_cell(gp_widget *self, gp_widget_table_cell *cell, unsigned int col_id)
{
	gp_widget_table_priv *tbl_priv = gp_widget_table_priv_get(self);

	switch (col_id) {
	case OBJECT:
		cell->text = table_data[tbl_priv->row_idx][0];
	break;
	case IS_FRUIT:
		cell->text = table_data[tbl_priv->row_idx][1];
	break;
	case NUMBER:
		cell->text = table_data[tbl_priv->row_idx][2];
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

	printf("Selected row %u\n", gp_widget_table_sel_get(ev->self));

	return 1;
}

gp_app_info app_info = {
	.name = "Table Example",
	.desc = "Table widget OPS example",
	.version = "1.0",
	.license = "GPL-2.0-or-later",
	.url = "http://gfxprim.ucw.cz",
	.authors = (gp_app_info_author []) {
		{.name = "Cyril Hrubis", .email = "metan@ucw.cz", .years = "2021-2023"},
		{}
	}
};

int main(int argc, char *argv[])
{
	gp_widget *layout = gp_widget_layout_json("table_example.json", NULL, NULL);
	if (!layout)
		return 0;

	gp_widgets_main_loop(layout, NULL, argc, argv);

	return 0;
}
