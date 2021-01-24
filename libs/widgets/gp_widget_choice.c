//SPDX-License-Identifier: LGPL-2.0-or-later

/*

   Copyright (c) 2014-2021 Cyril Hrubis <metan@ucw.cz>

 */

#include <gp_widgets.h>
#include <gp_string.h>

gp_widget *gp_widget_choice_new(unsigned int type,
                                const char *choices[],
                                unsigned int choice_cnt,
                                unsigned int selected,
				int (*on_event)(gp_widget_event *self),
				void *priv)
{
	size_t size = sizeof(struct gp_widget_choice)
	              + gp_string_arr_size(choices, choice_cnt);

	gp_widget *ret = gp_widget_new(type, GP_WIDGET_CLASS_CHOICE, size);
	if (!ret)
		return NULL;

	ret->choice->sel = selected;
	ret->choice->choices = gp_string_arr_copy(choices, choice_cnt, ret->choice->payload);
	ret->choice->max = choice_cnt;

	gp_widget_event_handler_set(ret, on_event, priv);

	return ret;
}

void gp_widget_choice_set(gp_widget *self, unsigned int sel)
{
	GP_WIDGET_CLASS_ASSERT(self, GP_WIDGET_CLASS_CHOICE, );

	if (self->choice->sel == sel)
		return;

	self->choice->sel = sel;

	gp_widget_redraw(self);
}

unsigned int gp_widget_choice_get(gp_widget *self)
{
	GP_WIDGET_CLASS_ASSERT(self, GP_WIDGET_CLASS_CHOICE, 0);

	return self->choice->sel;
}
