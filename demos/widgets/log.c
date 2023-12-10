//SPDX-License-Identifier: LGPL-2.0-or-later

/*

   Copyright (c) 2022-2023 Cyril Hrubis <metan@ucw.cz>

 */

#include <errno.h>
#include <stdio.h>
#include <widgets/gp_widgets.h>

static gp_widget *logs, *tbox;

int button_append(gp_widget_event *ev)
{
	if (ev->type != GP_WIDGET_EVENT_WIDGET)
		return 0;

	gp_widget_log_append(logs, gp_widget_tbox_text(tbox));

	return 1;
}

gp_app_info app_info = {
	.name = "Log Widget",
	.desc = "Log widget example",
	.version = "1.0",
	.license = "GPL-2.0-or-later",
	.url = "http://gfxprim.ucw.cz",
	.authors = (gp_app_info_author []) {
		{.name = "Cyril Hrubis", .email = "metan@ucw.cz", .years = "2022-2023"},
		{}
	}
};

int main(int argc, char *argv[])
{
	gp_htable *uids;

	gp_widget *layout = gp_widget_layout_json("log.json", NULL, &uids);
	if (!layout)
		return 0;

	logs = gp_widget_by_uid(uids, "logs", GP_WIDGET_LOG);
	tbox = gp_widget_by_uid(uids, "text", GP_WIDGET_TBOX);

	gp_htable_free(uids);

	gp_widgets_main_loop(layout, NULL, argc, argv);

	return 0;
}
