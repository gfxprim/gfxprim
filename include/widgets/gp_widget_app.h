//SPDX-License-Identifier: LGPL-2.0-or-later

/*

   Copyright (c) 2014-2020 Cyril Hrubis <metan@ucw.cz>

 */

#ifndef GP_WIDGET_APP_H__
#define GP_WIDGET_APP_H__

#include <widgets/gp_widget_types.h>

/**
 * @brief Loads a widget layout given application name.
 *
 * Looks for the layout in /etc/ and $HOME/.config/
 *
 * @app_name An application name, usually the same as the binary name.
 * @uids An pointer to store the hash table of UIDs to.
 *
 * @return An application widget layout or NULL in a case of failure.
 */
gp_widget *gp_app_layout_load(const char *app_name, void **uids);

/**
 * @brief Sets an application event hanlder.
 *
 * The application event handler works the same as a widget handler.
 *
 * @handler An pointer to application event handler or NULL to disable it.
 */
void gp_app_event_handler_set(int (*on_event)(gp_widget_event *ev));

#define GP_APP_DEFAULT_EVENT_MASK ( \
	(1<<GP_WIDGET_EVENT_FREE)   \
)

/**
 * @brief Masks out app event.
 *
 * @ev_type An event type to mask out.
 */
void gp_app_event_mask(enum gp_widget_event_type ev_type);

/**
 * @brief Unmasks an app event.
 *
 * @ev_type An event type to unmask.
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
 * This is used internally to send app events.
 *
 * @ev An widget event.
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

#endif /* GP_WIDGET_APP_H__ */
