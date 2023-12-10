//SPDX-License-Identifier: LGPL-2.0-or-later

/*

   Copyright (c) 2023 Cyril Hrubis <metan@ucw.cz>

 */

#include <gfxprim.h>

static void add_point(gp_widget *graph)
{
	static double x = 1.5;
	static double y = 0;
	x+=0.5;

	y = 0.99 * y + (0.2 * random()) / (1 << 30) - 0.2;

	gp_widget_graph_point_add(graph, x, y);
}

static uint32_t timer_cb(gp_timer *self)
{
	add_point(self->priv);

	return self->period;
}

static int graph_type_on_event(gp_widget_event *ev)
{
	gp_widget *graph = ev->self->priv;

	if (ev->type != GP_WIDGET_EVENT_WIDGET)
		return 0;

	size_t type = gp_widget_choice_sel_get(ev->self);

	gp_widget_graph_style_set(graph, type);

	return 0;
}

static gp_timer tmr = {
	.expires = 0,
	.period = 100,
	.callback = timer_cb,
	.id = "add point timer",
};

static int play_pause_on_event(gp_widget_event *ev)
{
	if (ev->type != GP_WIDGET_EVENT_WIDGET)
		return 0;

	if (gp_widget_button_type_get(ev->self) == GP_BUTTON_PAUSE) {
		gp_widgets_timer_rem(&tmr);
		gp_widget_button_type_set(ev->self, GP_BUTTON_PLAY);
	} else {
		gp_widgets_timer_ins(&tmr);
		gp_widget_button_type_set(ev->self, GP_BUTTON_PAUSE);
	}

	return 0;
}

static int max_y_on_event(gp_widget_event *ev)
{
	gp_widget *graph = ev->self->priv;

	if (ev->type != GP_WIDGET_EVENT_WIDGET)
		return 0;

	graph->graph->max_y = gp_widget_int_val_get(ev->self);
	gp_widget_redraw(graph);

	return 0;
}

static int min_y_on_event(gp_widget_event *ev)
{
	gp_widget *graph = ev->self->priv;

	if (ev->type != GP_WIDGET_EVENT_WIDGET)
		return 0;

	graph->graph->min_y = gp_widget_int_val_get(ev->self);
	gp_widget_redraw(graph);

	return 0;
}

gp_app_info app_info = {
	.name = "Graph Example",
	.desc = "Graph widget xample application",
	.version = "1.0",
	.license = "GPL-2.0-or-later",
	.url = "http://gfxprim.ucw.cz",
	.authors = (gp_app_info_author []) {
		{.name = "Cyril Hrubis", .email = "metan@ucw.cz", .years = "2023"},
		{}
	}
};

int main(int argc, char *argv[])
{
	gp_widget *graph = gp_widget_graph_new(GP_WIDGET_SIZE(400, 0, 0),
	                                       GP_WIDGET_SIZE(400, 0, 0),
	                                       NULL, NULL, 50);
	gp_widget *grid = gp_widget_grid_new(1, 2, 0);
	gp_widget *hbox = gp_widget_grid_new(6, 1, 0);
	gp_widget *graph_type = gp_widget_spinbutton_new(gp_widget_graph_style_names, GP_WIDGET_GRAPH_STYLE_MAX, 0);
	gp_widget *pause_btn = gp_widget_button_new(NULL, GP_BUTTON_PAUSE);
	gp_widget *max_y_spinner = gp_widget_spinner_new(1, 10, 5);
	gp_widget *min_y_spinner = gp_widget_spinner_new(-5, 0, 0);

	gp_widget_grid_no_border(hbox);

	gp_widget_grid_put(hbox, 0, 0, gp_widget_label_new("max y:", 0, 0));
	gp_widget_grid_put(hbox, 1, 0, max_y_spinner);
	gp_widget_grid_put(hbox, 2, 0, gp_widget_label_new("min y:", 0, 0));
	gp_widget_grid_put(hbox, 3, 0, min_y_spinner);
	gp_widget_grid_put(hbox, 4, 0, graph_type);
	gp_widget_grid_put(hbox, 5, 0, pause_btn);

	gp_widget_grid_put(grid, 0, 0, graph);
	gp_widget_grid_put(grid, 0, 1, hbox);

	gp_widget_grid_row_fill_set(grid, 1, 0);

	grid->align = GP_FILL;
	graph->align = GP_FILL;

	gp_widget_graph_yrange_set(graph, 0, 5);

	graph->graph->color = GP_WIDGETS_COL_BLUE;

	gp_widget_on_event_set(pause_btn, play_pause_on_event, NULL);
	gp_widget_on_event_set(graph_type, graph_type_on_event, graph);
	gp_widget_on_event_set(max_y_spinner, max_y_on_event, graph);
	gp_widget_on_event_set(min_y_spinner, min_y_on_event, graph);

	tmr.priv = graph;
	gp_widgets_timer_ins(&tmr);

	gp_widgets_main_loop(grid, NULL, argc, argv);

	return 0;
}
