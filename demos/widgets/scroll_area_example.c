//SPDX-License-Identifier: LGPL-2.0-or-later

/*

   Copyright (c) 2014-2020 Cyril Hrubis <metan@ucw.cz>

 */

#include <widgets/gp_widgets.h>

static void *uids;

int button_event(gp_widget_event *ev)
{
	if (ev->type == GP_WIDGET_EVENT_NEW)
		return 0;

	gp_widget *scroll = gp_widget_by_uid(uids, "area", GP_WIDGET_SCROLL_AREA);

	gp_coord dx = 0;
	gp_coord dy = 0;

	switch (ev->self->btn->label[0]) {
	case '^':
		dy = -1;
	break;
	case 'v':
		dy = 1;
	break;
	case '<':
		dx = -1;
	break;
	case '>':
		dx = 1;
	break;
	}

	printf("Scrolling by %i %i\n", dx, dy);

	gp_widget_scroll_area_move(scroll, dx, dy);

	return 1;
}

int inner_button_event(gp_widget_event *ev)
{
	if (ev->type == GP_WIDGET_EVENT_NEW)
		return 0;

	printf("Inner button clicked!\n");

	return 1;
}

int main(int argc, char *argv[])
{
	gp_widget *layout = gp_widget_layout_json("scroll_area_example.json", &uids);
	if (!layout)
		return 0;

	gp_widgets_main_loop(layout, "Scroll area example", NULL, argc, argv);

	return 0;
}
