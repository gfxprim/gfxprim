//SPDX-License-Identifier: LGPL-2.0-or-later

/*

   Copyright (c) 2014-2021 Cyril Hrubis <metan@ucw.cz>

 */

/**
 * @file gp_widget_app.h
 * @brief Application layout and events.
 */

#ifndef GP_WIDGET_APP_H
#define GP_WIDGET_APP_H

#include <widgets/gp_widget_types.h>

/**
 * @brief Loads a widget layout given application name.
 *
 * Looks for the layout in /etc/ and $HOME/.config/
 *
 * @param app_name An application name, usually the same as the binary name.
 * @param uids An pointer to store the hash table of UIDs to.
 *
 * @return An application widget layout or NULL in a case of failure.
 */
gp_widget *gp_app_layout_load(const char *app_name, gp_htable **uids);

/**
 * @brief Loads a widget layout given application name.
 *
 * Looks for the layout in /etc/ and $HOME/.config/
 *
 * @param app_name An application name, usually the same as the binary name.
 * @param callbacks Optional callbacks description, NULL if not used.
 * @param uids An pointer to store the hash table of UIDs to.
 *
 * @return An application widget layout or NULL in a case of failure.
 */
gp_widget *gp_app_layout_load2(const char *app_name,
                               const gp_widget_json_callbacks *const callbacks,
                               gp_htable **uids);

/**
 * @brief Loads an application fragment given application name and fragment name.
 *
 * Looks for the layout fragment in /etc/ and $HOME/.config/
 *
 * @param app_name An application name, usually the same as the binary name.
 * @param layout_name A layout name.
 * @param callbacks Optional callbacks description, NULL if not used.
 * @param uids An pointer to store the hash table of UIDs to.
 *
 * @return An application widget layout or NULL in a case of failure.
 */
gp_widget *gp_app_named_layout_load(const char *app_name, const char *layout_name,
                                    const gp_widget_json_callbacks *const callbacks,
                                    gp_htable **uids);


/**
 * @brief Sets an application event hanlder.
 *
 * The application event handler works the same as a widget handler.
 *
 * @param on_event An pointer to application event handler or NULL to disable
 *                 events.
 */
void gp_app_on_event_set(int (*on_event)(gp_widget_event *ev));

#define GP_APP_DEFAULT_EVENT_MASK ( \
	(1<<GP_WIDGET_EVENT_FREE)   \
)

/**
 * @brief Masks out app event.
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
 * This is used internally to send app events.
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

#endif /* GP_WIDGET_APP_H */
