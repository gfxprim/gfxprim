//SPDX-License-Identifier: LGPL-2.0-or-later

/*

   Copyright (c) 2014-2019 Cyril Hrubis <metan@ucw.cz>

 */

#include <widgets/gp_widgets.h>

int callback(gp_widget_event *ev)
{
	if (ev->type == GP_WIDGET_EVENT_NEW)
		return 0;

	switch (ev->self->type) {
	case GP_WIDGET_TBOX:
		if (ev->type != GP_WIDGET_EVENT_WIDGET)
			return 0;

		if (ev->sub_type == GP_WIDGET_TBOX_EDIT)
			printf("Text box edit '%s'\n", ev->self->tbox->buf);

		if (ev->sub_type == GP_WIDGET_TBOX_TRIGGER)
			printf("Text box enter\n");
	break;
	case GP_WIDGET_BUTTON:
		printf("Button pressed!\n");
	break;
	case GP_WIDGET_CHECKBOX:
		printf("Checkbox set to %i\n", ev->self->checkbox->val);
	break;
	}

	return 0;
}

int main(int argc, char *argv[])
{
	gp_widget *layout = gp_app_layout_load("tabs_example", NULL);
	if (!layout)
		return 0;

	gp_widgets_main_loop(layout, "Tabs example", NULL, argc, argv);

	return 0;
}
