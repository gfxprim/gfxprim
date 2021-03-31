//SPDX-License-Identifier: LGPL-2.0-or-later

/*

   Copyright (c) 2014-2020 Cyril Hrubis <metan@ucw.cz>

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
		printf("Username: '%s'\n", uname->tbox->buf);

	if (pass)
		printf("Password: '%s'\n", pass->tbox->buf);

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

	if (ev->self->b->val)
		pass->tbox->hidden = 0;
	else
		pass->tbox->hidden = 1;

	gp_widget_redraw(pass);

	return 0;
}

int main(int argc, char *argv[])
{
	const char *layout_path = "login_example_1.json";

	gp_widgets_getopt(&argc, &argv);

	if (argv[0])
		layout_path = "login_example_2.json";

	gp_widget *layout = gp_widget_layout_json(layout_path, &uids);
	if (!layout)
		return 0;

	gp_widgets_main_loop(layout, "Login!", NULL, 0, NULL);

	return 0;
}
