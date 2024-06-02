//SPDX-License-Identifier: LGPL-2.0-or-later

/*

   Copyright (c) 2014-2024 Cyril Hrubis <metan@ucw.cz>

 */

#include <stdio.h>
#include <string.h>
#include <widgets/gp_widget_ops.h>
#include <widgets/gp_widget_event.h>

static const char *ev_names[] = {
	[GP_WIDGET_EVENT_NEW] = "new",
	[GP_WIDGET_EVENT_FREE] = "free",
	[GP_WIDGET_EVENT_WIDGET] = "widget",
	[GP_WIDGET_EVENT_INPUT] = "input",
	[GP_WIDGET_EVENT_REDRAW] = "redraw",
	[GP_WIDGET_EVENT_RESIZE] = "resize",
	[GP_WIDGET_EVENT_COLOR_SCHEME] = "color_scheme",
};

void gp_widget_on_event_set(gp_widget *self,
                            int (*on_event)(gp_widget_event *), void *priv)
{
	GP_WIDGET_ASSERT(self, );

	self->on_event = on_event;
	self->priv = priv;
	self->no_events = 0;
}

void gp_widget_event_mask(gp_widget *self, enum gp_widget_event_type ev_type)
{
	if (!self) {
		GP_WARN("NULL widget!");
		return;
	}

	self->event_mask &= ~(1<<ev_type);
}

void gp_widget_event_unmask(gp_widget *self, enum gp_widget_event_type ev_type)
{
	if (!self) {
		GP_WARN("NULL widget!");
		return;
	}

	self->event_mask |= (1<<ev_type);
}

const char *gp_widget_event_type_name(enum gp_widget_event_type ev_type)
{
	if (ev_type >= GP_WIDGET_EVENT_MAX)
		return "invalid";

	if (ev_type < 0)
		return "invalid";

	return ev_names[ev_type];
}

void gp_widget_event_dump(gp_widget_event *ev)
{
	printf("Event type %s for widget %p type %s sub_type %u\n",
		gp_widget_event_type_name(ev->type),
		ev->self, gp_widget_type_id(ev->self),
		(unsigned int) ev->sub_type);

	switch (ev->type) {
	case GP_WIDGET_EVENT_INPUT:
		gp_ev_dump(ev->input_ev);
	break;
	default:
	break;
	}
}

int gp_widget_input_inject(gp_widget *self, gp_widget_event *ev)
{
	if (ev->type != GP_WIDGET_EVENT_INPUT)
		return 0;

	if (ev->input_ev->type == GP_EV_REL || ev->input_ev->type == GP_EV_ABS)
		return 0;

	return gp_widget_ops_event(self, gp_widgets_render_ctx(), ev->input_ev);
}
