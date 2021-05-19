//SPDX-License-Identifier: LGPL-2.0-or-later

/*

   Copyright (c) 2014-2020 Cyril Hrubis <metan@ucw.cz>

 */

/**
 * Widget events is an interface between the application and the widget
 * toolkit, typical event is a button press or a text edit. Each widget usually
 * has only one event callback and sends a subset of event types.
 */

#ifndef GP_WIDGET_EVENT_H
#define GP_WIDGET_EVENT_H

#include <stdarg.h>
#include <widgets/gp_widget_types.h>

/**
 * @brief Widget event type.
 *
 * We have 32bit mask so the maximal number of possible events is 32.
 */
enum gp_widget_event_type {
	/** Send right after widget has been allocated and initalized. */
	GP_WIDGET_EVENT_NEW,
	/** Send before widget is freed. */
	GP_WIDGET_EVENT_FREE,
	/** Widget specific event e.g. button has been pressed, details in sub_type */
	GP_WIDGET_EVENT_WIDGET,
	/** Raw user input event such as mouse movement or keypress. */
	GP_WIDGET_EVENT_INPUT,
	/** Send by pixmap widget when pixmap has has to be redrawn. */
	GP_WIDGET_EVENT_REDRAW,
	/** Send by pixmap widget when pixmap has to be resized */
	GP_WIDGET_EVENT_RESIZE,
	/** The number of events, i.e. last event + 1. */
	GP_WIDGET_EVENT_MAX,
};

#define GP_WIDGET_DEFAULT_EVENT_MASK ( \
	(1<<GP_WIDGET_EVENT_NEW) |     \
	(1<<GP_WIDGET_EVENT_FREE) |    \
	(1<<GP_WIDGET_EVENT_WIDGET)  \
)

/**
 * @brief Masks widget events
 *
 * Disables widget events.
 *
 * @self The widget
 * @ev_type Event type to disable
 */
void gp_widget_event_mask(gp_widget *self, enum gp_widget_event_type ev_type);

/**
 * @brief Unmasks widget events
 *
 * Enables widget events.
 *
 * @self The widget
 * @ev_type Event type to enable
 */
void gp_widget_event_unmask(gp_widget *self, enum gp_widget_event_type ev_type);

/**
 * @brief Returns string name for a given event type.
 *
 * @param ev_type Widget event type.
 * @return Widget event type name.
 */
const char *gp_widget_event_type_name(enum gp_widget_event_type ev_type);

/**
 * @brief Event structure holds all event parameters.
 */
struct gp_widget_event {
	struct gp_widget *self;
	/* generic event type, i.e. enum gp_widget_event_type */
	uint16_t type;
	/* widget specific subtype defined by widgets */
	uint16_t sub_type;
	/* internal DO NOT TOUCH */
	const struct gp_widget_render_ctx *ctx;
	union {
		void *ptr;
		long val;
		struct gp_bbox *bbox;
		struct gp_event *input_ev;
	};
};

/**
 * @brief Prints event details into stdout.
 *
 * @param ev Pointer to a widget event.
 */
void gp_widget_event_dump(gp_widget_event *ev);

/**
 * @brief Helper function to send a widget library event to application.
 *
 * @self Pointer to the widget sending this event.
 * @type Event type see gp_widget_event_type enum.
 * @return The return value from application event handler.
 */
static inline int gp_widget_send_event(gp_widget *self,
				       enum gp_widget_event_type type,
				       ...)
{
	if (!self->on_event)
		return 0;

	if (!(self->event_mask & (1<<type)))
		return 0;

	const struct gp_widget_render_ctx *ctx = NULL;

	va_list va;
	va_start(va, type);
	if (type == GP_WIDGET_EVENT_INPUT ||
	    type == GP_WIDGET_EVENT_RESIZE ||
	    type == GP_WIDGET_EVENT_REDRAW)
		ctx = va_arg(va, void*);
	long val = va_arg(va, long);
	va_end(va);

	gp_widget_event ev = {
		.self = self,
		.type = type,
		.ctx = ctx,
		.val = val,
	};

	return self->on_event(&ev);
}

/**
 * @brief Helper function to send a widget specific event to application.
 *
 * @self Pointer to the widget sending this event.
 * @type Event type as defined by a particular widget.
 * @return The return value from application event handler.
 */
static inline int gp_widget_send_widget_event(gp_widget *self,
				              unsigned int sub_type,
				              ...)
{
	if (!self->on_event)
		return 0;

	va_list va;
	va_start(va, sub_type);
	long val = va_arg(va, long);
	va_end(va);

	gp_widget_event ev = {
		.self = self,
		.type = GP_WIDGET_EVENT_WIDGET,
		.sub_type = sub_type,
		.val = val,
	};

	return self->on_event(&ev);
}

#endif /* GP_WIDGET_EVENT_H */
