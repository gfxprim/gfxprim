//SPDX-License-Identifier: LGPL-2.0-or-later

/*

   Copyright (c) 2021-2023 Cyril Hrubis <metan@ucw.cz>

 */

#include <widgets/gp_widgets.h>

static gp_widget *tabs;

static int button_add_callback(gp_widget_event *ev)
{
	static unsigned int counter;
	char buf[256];
	char label[64];

	if (ev->type != GP_WIDGET_EVENT_WIDGET)
		return 0;

	snprintf(buf, sizeof(buf), "#New Tab %u#\nThis is dynamically created tab!", counter);
	snprintf(label, sizeof(label), "Tab #%u", counter);

	counter++;

	gp_widget_tabs_tab_append(tabs, label, gp_widget_markup_new(buf, GP_MARKUP_GFXPRIM, 0));

	return 0;
}

static int button_rem_callback(gp_widget_event *ev)
{
	if (ev->type != GP_WIDGET_EVENT_WIDGET)
		return 0;

	gp_widget_tabs_tab_del(tabs, 0);

	return 0;
}

gp_app_info app_info = {
	.name = "Tabs",
	.desc = "Tabs widget example",
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
	gp_widget *layout = gp_widget_grid_new(1, 2, 0);
	gp_widget *buttons = gp_widget_grid_new(2, 1, 0);

	gp_widget_grid_no_border(buttons);

	tabs = gp_widget_tabs_new(0, 0, NULL, 0);

	gp_widget_grid_put(layout, 0, 0, tabs);
	gp_widget_grid_put(layout, 0, 1, buttons);
	gp_widget_grid_put(buttons, 0, 0,
	                   gp_widget_button_new2("tab", GP_BUTTON_ADD, button_add_callback, NULL));
	gp_widget_grid_put(buttons, 1, 0,
	                   gp_widget_button_new2("tab", GP_BUTTON_REM, button_rem_callback, NULL));

	gp_widgets_main_loop(layout, NULL, argc, argv);

	return 0;
}
