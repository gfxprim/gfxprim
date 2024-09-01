//SPDX-License-Identifier: LGPL-2.0-or-later

/*

   Copyright (c) 2014-2023 Cyril Hrubis <metan@ucw.cz>

 */

#include <widgets/gp_widgets.h>

static gp_htable *uids;

int login_callback(gp_widget_event *ev)
{
	if (ev->type != GP_WIDGET_EVENT_WIDGET)
		return 0;

	gp_widget *pass = gp_widget_by_uid(uids, "pass", GP_WIDGET_TBOX);
	gp_widget *uname = gp_widget_by_uid(uids, "uname", GP_WIDGET_TBOX);

	if (uname)
		printf("Username: '%s'\n", gp_widget_tbox_text(uname));

	if (pass)
		printf("Password: '%s'\n", gp_widget_tbox_text(pass));

	return 0;
}

int cancel_callback(gp_widget_event *ev)
{
	if (ev->type != GP_WIDGET_EVENT_WIDGET)
		return 0;

	exit(0);
}

int show_password(gp_widget_event *ev)
{
	gp_widget *pass = gp_widget_by_uid(uids, "pass", GP_WIDGET_TBOX);

	if (gp_widget_bool_get(ev->self))
		gp_widget_tbox_type_set(pass, GP_WIDGET_TBOX_NONE);
	else
		gp_widget_tbox_type_set(pass, GP_WIDGET_TBOX_HIDDEN);

	gp_widget_redraw(pass);

	return 0;
}

gp_app_info app_info = {
	.name = "Login!",
	.desc = "Login example",
	.version = "1.0",
	.license = "GPL-2.0-or-later",
	.url = "http://gfxprim.ucw.cz",
	.authors = (gp_app_info_author []) {
		{.name = "Cyril Hrubis", .email = "metan@ucw.cz", .years = "2014-2023"},
		{}
	}
};

int main(int argc, char *argv[])
{
	const char *layout_path = "login_example_1.json";

	gp_widgets_getopt(&argc, &argv);

	if (argv[0])
		layout_path = "login_example_2.json";

	gp_widget *layout = gp_widget_layout_json(layout_path, NULL, &uids);
	if (!layout)
		return 0;

	gp_widgets_main_loop(layout, NULL, 0, NULL);

	return 0;
}
