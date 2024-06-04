//SPDX-License-Identifier: LGPL-2.0-or-later

/*

   Copyright (c) 2014-2024 Cyril Hrubis <metan@ucw.cz>

 */

#include <stdio.h>
#include <string.h>
#include <widgets/gp_widget_ops.h>
#include <widgets/gp_widget_event.h>

void gp_widget_on_event_set(gp_widget *self,
                            int (*on_event)(gp_widget_event *), void *priv)
{
	GP_WIDGET_ASSERT(self, );

	self->on_event = on_event;
	self->priv = priv;
	self->no_events = 0;
}

void gp_widget_events_mask(gp_widget *self, enum gp_widget_event_type evs)
{
	GP_WIDGET_ASSERT(self, );

	self->event_mask &= ~evs;
}

void gp_widget_events_unmask(gp_widget *self, enum gp_widget_event_type evs)
{
	GP_WIDGET_ASSERT(self, );

	self->event_mask |= evs;
}

const char *gp_widget_event_type_name(enum gp_widget_event_type ev_type)
{
	switch (ev_type) {
	case GP_WIDGET_EVENT_NEW:
		return "new";
	case GP_WIDGET_EVENT_FREE:
		return "free";
	case GP_WIDGET_EVENT_WIDGET:
		return "widget";
	case GP_WIDGET_EVENT_INPUT:
		return "input";
	case GP_WIDGET_EVENT_REDRAW:
		return "redraw";
	case GP_WIDGET_EVENT_RESIZE:
		return "resize";
	case GP_WIDGET_EVENT_COLOR_SCHEME:
		return "color_scheme";
	default:
		return "invalid";
	}
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
