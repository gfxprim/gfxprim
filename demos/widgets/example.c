//SPDX-License-Identifier: GPL-2.0-or-later

/*

   Copyright (c) 2014-2022 Cyril Hrubis <metan@ucw.cz>

 */

#include <widgets/gp_widgets.h>

static gp_htable *uids;
static gp_widget *pbar;

int button_on_event(gp_widget_event *ev)
{
	if (ev->type != GP_WIDGET_EVENT_WIDGET)
		return 0;

	gp_widget *pass = gp_widget_by_uid(uids, "passwd", GP_WIDGET_TBOX);

	if (pass)
		printf("Password: %s\n", gp_widget_tbox_text(pass));

	return 1;
}

int slider_event(gp_widget_event *ev)
{
	if (ev->type != GP_WIDGET_EVENT_WIDGET)
		return 0;

	printf("Slider value=%"PRIi64"\n", gp_widget_int_val_get(ev->self));

	if (pbar)
		gp_widget_pbar_val_set(pbar, gp_widget_pbar_val_get(ev->self));

	return 0;
}

int pbar_event(gp_widget_event *ev)
{

	if (ev->type != GP_WIDGET_EVENT_WIDGET)
		return 0;

	printf("Progress bar value %"PRIu64"\n", gp_widget_pbar_val_get(ev->self));

	return 0;
}

gp_app_info app_info = {
	.name = "Example",
	.desc = "Example application",
	.version = "1.0",
	.license = "GPL-2.0-or-later",
	.url = "http://gfxprim.ucw.cz",
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

	pbar = gp_widget_by_uid(uids, "pbar", GP_WIDGET_PROGRESSBAR);

	gp_widgets_main_loop(layout, NULL, argc, argv);

	return 0;
}
