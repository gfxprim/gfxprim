//SPDX-License-Identifier: LGPL-2.0-or-later

/*

   Copyright (c) 2014-2022 Cyril Hrubis <metan@ucw.cz>

 */

#ifndef GP_WIDGET_KEYS
#define GP_WIDGET_KEYS

#include <input/gp_event.h>

/**
 * @brief Utility function to check for modifier keys.
 *
 * @ev An input event.
 * @return Non-zero if any of modifier keys is pressed such as control or alt.
 */
static inline int gp_widget_key_mod_pressed(gp_event *ev)
{
	return gp_event_any_key_pressed(ev, GP_KEY_LEFT_ALT, GP_KEY_RIGHT_ALT,
	                                    GP_KEY_LEFT_CTRL, GP_KEY_RIGHT_CTRL,
	                                    GP_KEY_LEFT_META, GP_KEY_RIGHT_META);
}

#endif /* GP_WIDGET_KEYS */
