//SPDX-License-Identifier: LGPL-2.0-or-later
/*

   Copyright (c) 2023 Cyril Hrubis <metan@ucw.cz>

 */

/*

   Minimal choice array example.

 */

#include <stdio.h>
#include <widgets/gp_widgets.h>

struct name_val {
	const char *name;
	int val;
};

static const struct name_val choices[] = {
	{"Choice 0", 0},
	{"Choice 10", 10},
	{"Choice 100", 100},
};

int spinner_on_event(gp_widget_event *ev)
{
	switch (ev->type) {
	case GP_WIDGET_EVENT_WIDGET:
		printf("Choice value %i\n", choices[gp_widget_int_val_get(ev->self)].val);
	break;
	}

	return 0;
}

int main(int argc, char *argv[])
{
	gp_widget *layout, *choice;

	choice = gp_widget_choice_arr_new(GP_WIDGET_RADIOBUTTON,
	                                  choices, GP_ARRAY_SIZE(choices),
	                                  sizeof(struct name_val), offsetof(struct name_val, name),
	                                  0, 0);

	gp_widget_on_event_set(choice, spinner_on_event, NULL);

	layout = gp_widget_grid_new(1, 1, 0);
	gp_widget_grid_put(layout, 0, 0, choice);

	if (!layout)
		return 0;

	gp_widgets_main_loop(layout, "Choices arr", NULL, argc, argv);

	return 0;
}
