//SPDX-License-Identifier: GPL-2.0-or-later

/*

   Copyright (c) 2014-2022 Cyril Hrubis <metan@ucw.cz>

 */

#include <widgets/gp_widgets.h>

static gp_htable *uids;

int button_on_event(gp_widget_event *ev)
{
	if (ev->type != GP_WIDGET_EVENT_WIDGET)
		return 0;

	gp_widget *pass = gp_widget_by_uid(uids, "passwd", GP_WIDGET_TBOX);

	if (pass)
		printf("Password: %s\n", pass->tbox->buf);

	return 1;
}

static gp_app_info app_info = {
	.name = "Example",
	.desc = "Example application",
	.version = "1.0",
	.license = "GPL-2.0-or-later",
	.url = "http://foo.bar",
	.authors = (gp_app_info_author []) {
		{.name = "Cyril Hrubis", .email = "metan@ucw.cz", .years = "2014-2022"},
		{}
	}
};

int main(int argc, char *argv[])
{
	gp_widget *layout = gp_widget_layout_json("example.json", NULL, &uids);
	if (!layout)
		return 0;

	gp_app_info_set(&app_info);

	gp_widgets_main_loop(layout, "Widgets Example", NULL, argc, argv);

	return 0;
}
