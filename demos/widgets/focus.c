//SPDX-License-Identifier: LGPL-2.0-or-later

/*

   Copyright (c) 2014-2019 Cyril Hrubis <metan@ucw.cz>

 */

#include <gfxprim.h>

int move_focus(gp_widget_event *ev)
{
	if (ev->type != GP_WIDGET_EVENT_WIDGET)
		return 0;

	gp_widget_focus_set(ev->self->priv);

	return 0;
}

int main(int argc, char *argv[])
{
	gp_widget *btn1 = gp_widget_button_new("Move focus", 0, NULL, NULL);
	gp_widget *btn2 = gp_widget_button_new("Move focus", 0, NULL, NULL);
	gp_widget *grid = gp_widget_grid_new(2, 1, 0);

	gp_widget_event_handler_set(btn1, move_focus, btn2);
	gp_widget_event_handler_set(btn2, move_focus, btn1);

	gp_widget_grid_put(grid, 0, 0, btn1);
	gp_widget_grid_put(grid, 1, 0, btn2);

	gp_widgets_main_loop(grid, "Focus", NULL, argc, argv);

	return 0;
}
