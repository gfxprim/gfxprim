//SPDX-License-Identifier: LGPL-2.0-or-later

/*

   Copyright (c) 2014-2024 Cyril Hrubis <metan@ucw.cz>

 */

/**
 * @file gp_widget_checkbox.h
 * @brief A checkbox widget.
 *
 * Checkbox is a bool class widget, after it's created it's acessed by the
 * class functions such as gp_widget_bool_set().
 *
 * @image html checkbox.png
 *
 * Checkbox widget JSON attributes
 * -------------------------------
 *
 * | Attribute |  Type  | Default | Description                                     |
 * |-----------|--------|---------|-------------------------------------------------|
 * | **label** | string |         | A checkbox label, can be NULL if there is none. |
 * |  **set**  |  bool  |  False  | Initial checkbox value.                         |
 */

#ifndef GP_WIDGET_CHECKBOX_H
#define GP_WIDGET_CHECKBOX_H

#include <widgets/gp_widget_class_bool.h>

/**
 * @brief Allocate and initialize new checkbox widget.
 *
 * @param label A checkbox label.
 * @param val Initial checkbox value.
 *
 * @return A checkbox widget.
 */
gp_widget *gp_widget_checkbox_new(const char *label, bool set);

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
static inline gp_widget *gp_widget_checkbox_new2(const char *label, bool set,
                                                 int (*on_event)(gp_widget_event *ev),
                                                 void *priv)
{
	gp_widget *ret = gp_widget_checkbox_new(label, set);

	gp_widget_on_event_set(ret, on_event, priv);

	return ret;
}

/**
 * @brief Gets checkbox label.
 *
 * @param self A checkbox widget.
 * @return A checkbox label or NULL if there is none.
 */
const char *gp_widget_checkbox_label_get(gp_widget *self);

#endif /* GP_WIDGET_CHECKBOX_H */
