//SPDX-License-Identifier: LGPL-2.0-or-later
/*

   Copyright (c) 2021 Cyril Hrubis <metan@ucw.cz>

 */

/*

   Minimal choice ops example.

 */

#include <stdio.h>
#include <widgets/gp_widgets.h>

static int choice_cnt = 1;
static int choice_sel = 0;

static const char *choices[] = {
	"Choice A",
	"Choice B",
	"Choice XXX",
};

static gp_widget *choice_widget;

static const char *choices_get_choice(gp_widget *self, size_t idx)
{
	(void) self;

	return choices[idx];
}

static size_t choices_get(gp_widget *self, enum gp_widget_choice_op op)
{
	(void) self;

	switch (op) {
	case GP_WIDGET_CHOICE_OP_SEL:
		return choice_sel;
	case GP_WIDGET_CHOICE_OP_CNT:
		return choice_cnt;
	}

	return 0;
}

static void choices_set(gp_widget *self, enum gp_widget_choice_op op, size_t val)
{
	(void) self;

	if (op != GP_WIDGET_CHOICE_OP_SEL)
		return;

	choice_sel = val;
}

int spinner_on_event(gp_widget_event *ev)
{
	switch (ev->type) {
	case GP_WIDGET_EVENT_NEW:
		gp_widget_int_set(ev->self, 0, GP_ARRAY_SIZE(choices), choice_cnt);
	break;
	case GP_WIDGET_EVENT_WIDGET:
		choice_cnt = gp_widget_int_val_get(ev->self);
		gp_widget_choice_refresh(choice_widget);
	break;
	}

	return 0;
}

struct gp_widget_choice_ops choices_ops = {
	.get_choice = choices_get_choice,
	.get = choices_get,
	.set = choices_set,
};

int main(int argc, char *argv[])
{
	gp_htable *uids;

	gp_widget *layout = gp_widget_layout_json("choice.json", &uids);
	if (!layout)
		return 0;

	choice_widget = gp_widget_by_cuid(uids, "choice", GP_WIDGET_CLASS_CHOICE);

	gp_htable_free(uids);

	gp_widgets_main_loop(layout, "Choices OPS", NULL, argc, argv);

	return 0;
}
