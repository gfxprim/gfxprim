//SPDX-License-Identifier: LGPL-2.0-or-later

/*

   Copyright (c) 2014-2023 Cyril Hrubis <metan@ucw.cz>

 */

/**
 * @file gp_widget_checkbox.h
 * @brief A checkbox widget.
 *
 * @image html checkbox.png
 */

#ifndef GP_WIDGET_CHECKBOX_H
#define GP_WIDGET_CHECKBOX_H

#include <widgets/gp_widget_bool.h>

/**
 * @brief Allocate and initialize new checkbox widget.
 *
 * @param label A checkbox label.
 * @param val Initial checkbox value.
 *
 * @return A checkbox widget.
 */
gp_widget *gp_widget_checkbox_new(const char *label, int val);

/**
 * @brief Allocate and initialize new checkbox widget.
 *
 * @param label A checkbox label.
 * @param val Initial checkbox value.
 * @param on_event An widget event handler.
 * @param priv User private data.
 *
 * @return A checkbox widget.
 */
static inline gp_widget *gp_widget_checkbox_new2(const char *label, int val,
                                                 int (*on_event)(gp_widget_event *ev),
                                                 void *priv)
{
	gp_widget *ret = gp_widget_checkbox_new(label, val);

	gp_widget_on_event_set(ret, on_event, priv);

	return ret;
}

/**
 * @brief Sets a checkbox value.
 *
 * @param self A checkbox widget.
 * @param val New checkbox value.
 */
void gp_widget_checkbox_set(gp_widget *self, int val);

/**
 * @brief Toggles a checkbox value.
 *
 * @param self A checkbox widget.
 */
void gp_widget_checkbox_toggle(gp_widget *self);


/**
 * @brief Returns a checkbox value.
 *
 * @param self A checkbox widget.
 *
 * @return Returns checkbox value i.e. 1 or 0. If widget passes as self is not
 *         checkbox -1 is returned.
 */
int gp_widget_checkbox_get(gp_widget *self);

#endif /* GP_WIDGET_CHECKBOX_H */
