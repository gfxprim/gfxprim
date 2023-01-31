//SPDX-License-Identifier: LGPL-2.0-or-later

/*

   Copyright (c) 2020 Cyril Hrubis <metan@ucw.cz>

 */

#include <gfxprim.h>

static int disable(gp_widget_event *ev)
{
	if (ev->type != GP_WIDGET_EVENT_WIDGET)
		return 0;

	gp_widget_disable(ev->self->priv);

	return 0;
}

static int enable(gp_widget_event *ev)
{
	if (ev->type != GP_WIDGET_EVENT_WIDGET)
		return 0;

	gp_widget_enable(ev->self->priv);

	return 0;
}

int main(int argc, char *argv[])
{
	gp_widget *btn_disable = gp_widget_button_new("Disable", 0);
	gp_widget *btn_enable = gp_widget_button_new("Enable", 0);
	gp_widget *buttons = gp_widget_grid_new(2, 1, 0);
	gp_widget *grid = gp_widget_grid_new(1, 2, 0);
	const char *choices[] = {
		"Choice #1",
		"Choice #2",
		"Choice #3"
	};
	gp_widget *choice = gp_widget_radiobutton_new(choices, 3, 0);
	gp_widget *frame = gp_widget_frame_new("Target widget", 0, choice);

	gp_widget_on_event_set(btn_disable, disable, frame);
	gp_widget_on_event_set(btn_enable, enable, frame);

	gp_widget_grid_border_set(buttons, 0, 0);

	gp_widget_grid_put(buttons, 0, 0, btn_disable);
	gp_widget_grid_put(buttons, 1, 0, btn_enable);

	gp_widget_grid_put(grid, 0, 0, frame);
	gp_widget_grid_put(grid, 0, 1, buttons);

	gp_widgets_main_loop(grid, "Disable", NULL, argc, argv);

	return 0;
}
