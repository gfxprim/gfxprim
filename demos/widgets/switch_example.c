//SPDX-License-Identifier: LGPL-2.0-or-later

/*

   Copyright (c) 2014-2020 Cyril Hrubis <metan@ucw.cz>

 */

#include <widgets/gp_widgets.h>

static int set_layout0(gp_widget_event *ev)
{
	if (ev->type != GP_WIDGET_EVENT_WIDGET)
		return 0;

	gp_widget_switch_layout(ev->self->priv, 0);
	return 1;
}

static int set_layout1(gp_widget_event *ev)
{
	if (ev->type != GP_WIDGET_EVENT_WIDGET)
		return 0;

	gp_widget_switch_layout(ev->self->priv, 1);
	return 1;
}

static int set_layout2(gp_widget_event *ev)
{
	if (ev->type != GP_WIDGET_EVENT_WIDGET)
		return 0;

	gp_widget_switch_layout(ev->self->priv, 2);
	return 1;
}

static int keypress_handler(gp_widget_event *ev)
{
	if (ev->type != GP_WIDGET_EVENT_INPUT)
		return 0;

	if (ev->input_ev->type != GP_EV_KEY)
		return 0;

	if (ev->input_ev->code != 1)
		return 0;

	switch (ev->input_ev->val) {
	case GP_KEY_LEFT:
		gp_widget_switch_move(ev->self->priv, -1);
		return 1;
	break;
	case GP_KEY_RIGHT:
		gp_widget_switch_move(ev->self->priv, 1);
		return 1;
	break;
	}

	return 0;
}

int main(int argc, char *argv[])
{
	gp_widget *layout = gp_widget_switch_new(3);
	gp_widget *markup = gp_widget_markup_new("#Layout \\#2#\n\nThere is nothing on the next one.\nUse arrow keys to move.", GP_MARKUP_GFXPRIM, 0);
	gp_widget *markup_grid = gp_widget_grid_new(1, 2, 0);

	gp_widget *btn_grid = gp_widget_grid_new(2, 1, 0);
	gp_widget_grid_no_border(btn_grid);
	gp_widget_grid_put(btn_grid, 0, 0, gp_widget_button_new2("Prev", GP_BUTTON_PREV, set_layout0, layout));
	gp_widget_grid_put(btn_grid, 1, 0, gp_widget_button_new2("Next", GP_BUTTON_NEXT, set_layout2, layout));

	gp_widget_grid_put(markup_grid, 0, 0, markup);
	gp_widget_grid_put(markup_grid, 0, 1, btn_grid);

	gp_widget_switch_put(layout, 0, gp_widget_button_new2("Switch layout", 0, set_layout1, layout));
        gp_widget_switch_put(layout, 1, markup_grid);

	gp_widget_on_event_set(layout, keypress_handler, layout);
	gp_widget_event_unmask(layout, GP_WIDGET_EVENT_INPUT);

	gp_widgets_main_loop(layout, NULL, argc, argv);

	return 0;
}
