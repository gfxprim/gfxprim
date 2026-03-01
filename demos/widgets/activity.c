//SPDX-License-Identifier: LGPL-2.0-or-later
/*

   Copyright (c) 2026 Cyril Hrubis <metan@ucw.cz>

 */

/*

   Minimal activity widget example.

 */

#include <stdio.h>
#include <widgets/gp_widgets.h>

gp_app_info app_info = {
	.name = "Activity",
	.desc = "Activity example",
	.version = "1.0",
	.license = "GPL-2.0-or-later",
	.url = "http://gfxprim.ucw.cz",
	.authors = (gp_app_info_author []) {
		{.name = "Cyril Hrubis", .email = "metan@ucw.cz", .years = "2026"},
		{}
	}
};

static int button_on_event_start(gp_widget_event *ev)
{
	if (ev->type != GP_WIDGET_EVENT_WIDGET)
		return 0;

	if (ev->sub_type != GP_WIDGET_BUTTON_TRIGGER)
		return 0;

	gp_widget_activity_start(ev->self->priv);
	return 1;
}

static int button_on_event_stop(gp_widget_event *ev)
{
	if (ev->type != GP_WIDGET_EVENT_WIDGET)
		return 0;

	if (ev->sub_type != GP_WIDGET_BUTTON_TRIGGER)
		return 0;

	gp_widget_activity_stop(ev->self->priv);
	return 1;
}

int main(int argc, char *argv[])
{
	gp_widget *layout = gp_widget_vbox_new(2, 0);
	gp_widget *buttons = gp_widget_hbox_new(2, 0);
	gp_widget *activity = gp_widget_activity_new("Working ...");
	gp_widget *button_start = gp_widget_button_new("Start", 0);
	gp_widget *button_stop = gp_widget_button_new("Stop", 0);

	layout->align = GP_FILL;

	gp_widget_box_put(layout, 0, activity);
	gp_widget_box_put(layout, 1, buttons);

	gp_widget_on_event_set(button_start, button_on_event_start, activity);
	gp_widget_on_event_set(button_stop, button_on_event_stop, activity);

	gp_widget_box_put(buttons, 0, button_start);
	gp_widget_box_put(buttons, 1, button_stop);

	gp_widgets_main_loop(layout, NULL, argc, argv);

	return 0;
}
