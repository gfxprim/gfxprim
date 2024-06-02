//SPDX-License-Identifier: LGPL-2.0-or-later

/*

   Copyright (c) 2014-2024 Cyril Hrubis <metan@ucw.cz>

 */

/**
 * @file gp_app_event.h
 * @brief Application events.
 *
 * Example app event handler
 * -------------------------
 * @code
 * static void save_app_state(void)
 * {
 *	//Save application state to a file here
 * }
 *
 * static int app_on_event(gp_widget_event *ev)
 * {
 *	if (ev->type == GP_WIDGET_EVENT_FREE)
 *		save_app_state();
 * }
 *
 * ...
 *
 * int main(void)
 * {
 *	...
 *	gp_app_on_event_set(app_on_event);
 *	...
 * }
 * @endcode
 */

#ifndef GP_APP_EVENT_H
#define GP_APP_EVENT_H

#include <widgets/gp_widget_event.h>

/**
 * @brief Sets an application event hanlder.
 *
 * The application event handler works the same as a gp_widget::on_event() handler.
 *
 * @attention Not all events are enabled by default, see #GP_APP_EVENT_DEFAULT_MASK.
 *
 * Currently application can receive these events:
 *
 * **GP_WIDGET_EVENT_FREE**
 *
 * This event is send when gp_widgets_exit() is called to exit the
 * gp_widgets_main_loop().
 *
 * **GP_WIDGET_EVENT_INPUT**
 *
 * Routes any input events unused by the widget layout to the application
 * handler.
 *
 * @attention Note that which events are consumed by the widget layout depends
 *            on which widget in the current layout is focused.
 *
 * **GP_WIDGET_EVENT_COLOR_SCHEME**
 *
 * See #gp_widgets_color_scheme and gp_widgets_color_scheme_set() for details.
 *
 * @param on_event An pointer to application event handler or NULL to disable
 *                 events.
 */
void gp_app_on_event_set(int (*on_event)(gp_widget_event *ev));

/** @brief Default app event mask. */
#define GP_APP_EVENT_DEFAULT_MASK (1<<GP_WIDGET_EVENT_FREE)

/**
 * @brief Masks out an app event.
 *
 * @param ev_type An event type to mask out.
 */
void gp_app_event_mask(enum gp_widget_event_type ev_type);

/**
 * @brief Unmasks an app event.
 *
 * @param ev_type An event type to unmask.
 */
void gp_app_event_unmask(enum gp_widget_event_type ev_type);

struct gp_app {
	uint32_t ev_mask;
	int (*on_event)(gp_widget_event *ev);
};

extern const struct gp_app *gp_app;

/**
 * @brief Sends an event to the application.
 *
 * This is called by the widget library when app event should be send.
 *
 * @param ev_type An widget event.
 * @param ... Optional pointer or long integer.
 *
 * @return Non-zero if event was handled, zero otherwise.
 */
static inline int gp_app_send_event(enum gp_widget_event_type ev_type, ...)
{
	va_list va;
	long val;

	if (!gp_app->on_event)
		return 0;

	if (!(gp_app->ev_mask & (1<<ev_type)))
		return 0;

	va_start(va, ev_type);
	val = va_arg(va, long);
	va_end(va);

	gp_widget_event ev = {
		.type = ev_type,
		.val = val,
	};

	return gp_app->on_event(&ev);
}

#endif /* GP_APP_EVENT_H */
