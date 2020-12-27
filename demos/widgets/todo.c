//SPDX-License-Identifier: LGPL-2.0-or-later
/*
 * Copyright (c) 2020 Richard Palethorpe <richiejp@f-m.fm>
 *
 * Classic TODO application.
 *
 * Basic Features:
 *
 * + Add new TODO text item (single unformatted line of text)
 * + Delete TODO
 * + List TODOs
 * + Edit TODO
 * + Toggle TODO as done
 */

#include <widgets/gp_widgets.h>

#define TODO_MAX 25

typedef struct todo {
	int active:1;
	int done:1;
	char text[TODO_MAX];
} todo;

static todo todos[TODO_MAX] = {
	{ 1, 0, "Handle ENOMEM :-)" },
	{ 1, 0, "Add way to delete TODOs" },
	{ 0 },
};

static gp_widget *grid;

static void widget_todo_add(int t)
{
	gp_widget_grid_add_row(grid);
	gp_widget_grid_put(grid, 0, 1 + t,
			   gp_widget_tbox_new(todos[t].text, TODO_MAX, 0, 0, 0, 0));
	gp_widget_grid_put(grid, 1, 1 + t,
			   gp_widget_checkbox_new(0, todos[t].done, NULL, NULL));
}

static int on_new(gp_widget_event *ev)
{
	int i = 0;
	todo *t;

	if (ev->type != GP_WIDGET_EVENT_ACTION)
		return 0;

	while (todos[i].active)
		i++;

	t = &todos[i];
	t->active = 1;
	t->done = 0;
	t->text[0] = '\0';

	widget_todo_add(i);

	return 1;
}

int main(int argc, char *argv[])
{
	int i;
	gp_widget *outer = gp_widget_grid_new(1, 2);

	outer->align = GP_HFILL | GP_TOP;

	gp_widget_grid_put(outer, 0, 0, gp_widget_button_new("New", on_new, NULL));

	grid = gp_widget_grid_new(2, 1);
	gp_widget_grid_put(outer, 0, 1, grid);

	grid->align = GP_HFILL | GP_TOP;

	gp_widget_grid_put(grid, 0, 0, gp_widget_label_new("Description", 0, 1));
	gp_widget_grid_put(grid, 1, 0, gp_widget_label_new("Done?", 0, 1));

	for (i = 0; todos[i].active; i++)
		widget_todo_add(i);

	gp_widgets_main_loop(outer, "TODO", NULL, argc, argv);

	return 0;
}
