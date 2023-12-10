//SPDX-License-Identifier: LGPL-2.0-or-later

/*

   Copyright (c) 2014-2019 Cyril Hrubis <metan@ucw.cz>

 */

#include <widgets/gp_widgets.h>

static int button_callback(gp_widget_event *ev)
{
	if (ev->type != GP_WIDGET_EVENT_WIDGET)
		return 0;

	gp_widget *layout = ev->self->priv;

	gp_widget *w1 = gp_widget_grid_rem(layout, 0, 0);
	gp_widget *w2 = gp_widget_grid_rem(layout, 1, 0);

	gp_widget_grid_put(layout, 0, 0, w2);
	gp_widget_grid_put(layout, 1, 0, w1);

	return 0;
}

int main(int argc, char *argv[])
{
	gp_widget *layout = gp_widget_grid_new(2, 2, 0);

	layout->align = GP_FILL;

	gp_widget* button_ok = gp_widget_button_new2("OK", 0, button_callback, layout);

	gp_widget_grid_put(layout, 0, 0, gp_widget_label_new("Text Label", 0, 0));
	gp_widget_grid_put(layout, 0, 1, button_ok);

	gp_widgets_main_loop(layout, NULL, argc, argv);

	return 0;
}
