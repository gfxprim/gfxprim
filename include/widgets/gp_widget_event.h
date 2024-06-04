//SPDX-License-Identifier: LGPL-2.0-or-later

/*

   Copyright (c) 2014-2024 Cyril Hrubis <metan@ucw.cz>

 */

/**
 * @file gp_widget_event.h
 * @brief Widget event handling
 *
 * Widget events is an interface between the application and the widget
 * toolkit, typical event is a button press or a text edit. Each widget can
 * have only one event callback and sends a subset of event types.
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
	/**
	 * @brief Widget was created and initialized.
	 *
	 * This event is send right after widget has been allocated and
	 * initalized in the JSON parser. The intended purpose is to be able to
	 * be able to check parameters or finish initialization of widgets
	 * loaded from a JSON layout.
	 */
	GP_WIDGET_EVENT_NEW = 0x01,
	/**
	 * @brief Widget is about to be freed.
	 *
	 * See gp_widget_free() for details.
	 */
	GP_WIDGET_EVENT_FREE = 0x02,
	/**
	 * @brief Widget specific event.
	 *
	 * E.g. button has been pressed, each widget defines its enum of events
	 * and these are passed in the #gp_widget_event::sub_type.
	 */
	GP_WIDGET_EVENT_WIDGET = 0x04,
	/**
	 * @brief An input event.
	 *
	 * Raw #gp_event input event such as mouse movement or keypress.
	 *
	 * @attention The #gp_event::st cursor position is normalized so that
	 *            0,0 is top left corner of the widget.
	 *
	 * The event handler must return non-zero if the event was used and
	 * non-zero otherwise.
	 */
	GP_WIDGET_EVENT_INPUT = 0x08,
	/**
	 * @brief Pixmap redraw event.
	 *
	 * TODO: Move to pixmap specific event?
	 *
	 * Send by pixmap widget when pixmap has has to be redrawn.
	 */
	GP_WIDGET_EVENT_REDRAW = 0x10,
	/**
	 * @brief Widget was resized.
	 *
	 * Send when widget was resized.
	 */
	GP_WIDGET_EVENT_RESIZE = 0x20,
	/**
	 * @brief A color scheme has changed.
	 *
	 * See #gp_widgets_color_scheme and gp_widgets_color_scheme_set() for
	 * details.
	 */
	GP_WIDGET_EVENT_COLOR_SCHEME = 0x40,
	/**
	 * @brief Default widget event mask.
	 *
	 * This is the default mask for newly created widgets.
	 */
	GP_WIDGET_EVENT_DEFAULT_MASK = GP_WIDGET_EVENT_NEW | GP_WIDGET_EVENT_FREE | GP_WIDGET_EVENT_WIDGET,
};

/**
 * @brief Sets a widget event handler.
 *
 * Note that even after setting event handler certain widget events has to be
 * unmasked with gp_widget_event_unmask() in order to receive them.
 *
 * @param self A widget.
 * @param on_event An widget event handler.
 * @param priv An user pointer stored in the widget.
 */
void gp_widget_on_event_set(gp_widget *self,
                            int (*on_event)(gp_widget_event *), void *priv);

/**
 * @brief Masks widget event.
 *
 * Disables a widget event(s).
 *
 * @param self A widget.
 * @param evs A bitmask of events to disable.
 */
void gp_widget_events_mask(gp_widget *self, enum gp_widget_event_type evs);

/**
 * @brief Unmasks widget event.
 *
 * Enables a widget event(s).
 *
 * @param self A widget.
 * @param evs A bitmask of events to enable.
 */
void gp_widget_events_unmask(gp_widget *self, enum gp_widget_event_type evs);

/**
 * @brief Returns string name for a given event type.
 *
 * @param ev Widget event type.
 * @return Widget event type name.
 */
const char *gp_widget_event_type_name(enum gp_widget_event_type ev);

/**
 * @brief Event structure passed to widget event handler.
 */
struct gp_widget_event {
	/** @brief The widget the event is for. */
	gp_widget *self;
	/** @brief An event type, enum gp_widget_event_type. */
	uint16_t type;
	/**
	 * @brief Widget specific subtype defined by widgets.
	 *
	 * For all widget specific events the type is set to
	 * GP_WIDGET_EVENT_WIDGET and the sub_type is documented for each
	 * particular widget, e.g. #gp_widget_tbox_event_type.
	 */
	uint16_t sub_type;
	/* internal DO NOT TOUCH */
	const struct gp_widget_render_ctx *ctx;
	union {
		/** @brief Optional pointer. */
		void *ptr;
		/** @brief Optional integer value. */
		long val;
		/** @brief Optional bounding box. */
		struct gp_bbox *bbox;
		/** @brief An input event, set for GP_WIDGET_EVENT_INPUT */
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
 * This is called by the widget library when event should be send to the
 * widget.
 *
 * @param self Pointer to the widget sending this event.
 * @param type Event type see gp_widget_event_type enum.
 * @return The return value from application event handler.
 */
static inline int gp_widget_send_event(gp_widget *self,
				       enum gp_widget_event_type type,
				       ...)
{
	if (!self->on_event)
		return 0;

	if (!(self->event_mask & type))
		return 0;

	const struct gp_widget_render_ctx *ctx = NULL;

	va_list va;
	va_start(va, type);
	if (type == GP_WIDGET_EVENT_INPUT ||
	    type == GP_WIDGET_EVENT_RESIZE ||
	    type == GP_WIDGET_EVENT_REDRAW ||
	    type == GP_WIDGET_EVENT_COLOR_SCHEME)
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
 * This is called by the widget library when event should be send from a widget
 * to the application. This function fills in the #gp_widget_event structure
 * and then calls the gp_widget::on_event() function.
 *
 * @param self Pointer to the widget sending this event.
 *
 * @param sub_type Event subtype as defined by a particular widget e.g.
 * #gp_widget_tbox_event_type.
 * @param ... An optional pointer or long integer value.
 *
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

/**
 * @brief A helper function to inject key and utf input events to a widget.
 *
 * Internal function used in widget tests.
 *
 * This function takes a widget event and if the event type is
 * GP_WIDGET_EVENT_INPUT the input event is injected to the widget input event
 * handler, i.e. the widget will get the input as if it was focused.
 *
 * This of course works only for key presses and unicode input events as
 * relative or absolute coordinates are normalized as events pass down the
 * widget tree, i.e. for any event widget receives the widget top level corner
 * has coordinate [0,0]. Hence these kinds of events are not injected.
 *
 * @param self A widget to inject the input event to.
 * @param ev A widget event.
 *
 * @return Non-zero if the event was handled, zero otherwise.
 */
int gp_widget_input_inject(gp_widget *self, gp_widget_event *ev);

#endif /* GP_WIDGET_EVENT_H */
