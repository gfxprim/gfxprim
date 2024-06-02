//SPDX-License-Identifier: LGPL-2.0-or-later

/*

   Copyright (c) 2014-2024 Cyril Hrubis <metan@ucw.cz>

 */

#include <core/gp_debug.h>
#include <widgets/gp_widget.h>
#include <widgets/gp_app_event.h>

static struct gp_app app = {
	.ev_mask = GP_APP_EVENT_DEFAULT_MASK,
};

const struct gp_app *gp_app = &app;

void gp_app_event_mask(enum gp_widget_event_type ev_type)
{
	app.ev_mask &= ~(1<<ev_type);
}

void gp_app_event_unmask(enum gp_widget_event_type ev_type)
{
	app.ev_mask |= (1<<ev_type);
}

void gp_app_on_event_set(int (*on_event)(gp_widget_event *ev))
{
	app.on_event = on_event;
}
