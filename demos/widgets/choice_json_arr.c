//SPDX-License-Identifier: LGPL-2.0-or-later
/*

   Copyright (c) 2023 Cyril Hrubis <metan@ucw.cz>

 */

/*

   Minimal choice JSON array example.

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

int choice_on_event(gp_widget_event *ev)
{
	switch (ev->type) {
	case GP_WIDGET_EVENT_WIDGET:
		printf("Choice value %i\n", choices[gp_widget_int_val_get(ev->self)].val);
	break;
	}

	return 0;
}

const gp_widget_choice_desc choice_desc = {
	.ops = &gp_widget_choice_arr_ops,
	.arr = &(gp_widget_choice_arr){
		.ptr = choices,
		.memb_cnt = GP_ARRAY_SIZE(choices),
		.memb_size = sizeof(struct name_val),
		.memb_off = offsetof(struct name_val, name),
	},
};

gp_app_info app_info = {
	.name = "Choice JSON array",
	.desc = "Choice JSON array example",
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
	gp_widget *layout = gp_widget_layout_json("choice_json_arr.json", NULL, NULL);
	if (!layout)
		return 0;

	gp_widgets_main_loop(layout, NULL, argc, argv);

	return 0;
}
