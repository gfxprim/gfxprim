//SPDX-License-Identifier: LGPL-2.0-or-later

/*

   Copyright (c) 2014-2019 Cyril Hrubis <metan@ucw.cz>

 */

#include <widgets/gp_widgets.h>

static void *uids;

int button_callback(gp_widget_event *ev)
{
	if (ev->type == GP_WIDGET_EVENT_NEW)
		return 0;

	gp_widget *pass = gp_widget_by_uid(uids, "passwd", GP_WIDGET_TEXTBOX);

	if (pass)
		printf("Password: %s\n", pass->tbox->buf);

	return 1;
}

int main(int argc, char *argv[])
{
	gp_widget *layout = gp_app_layout_load("example", &uids);
	if (!layout)
		return 0;

	gp_widgets_main_loop(layout, "Widgets Example", NULL, argc, argv);

	return 0;
}
