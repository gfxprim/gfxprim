//SPDX-License-Identifier: LGPL-2.0-or-later

/*

   Copyright (c) 2014-2020 Cyril Hrubis <metan@ucw.cz>

 */

#ifndef GP_WIDGET_CHECKBOX_H__
#define GP_WIDGET_CHECKBOX_H__

#include <widgets/gp_widget_bool.h>

/**
 * @brief Allocate and initialize new checkbox widget.
 *
 * @label A checkbox label.
 * @val Initial checkbox value.
 *
 * @return A checkbox widget.
 */
gp_widget *gp_widget_checkbox_new(const char *label, int val,
                                  int on_event(gp_widget_event *ev),
                                  void *priv);

/**
 * @brief Sets a checkbox value.
 *
 * @self A checkbox widget.
 * @val New checkbox value.
 */
void gp_widget_checkbox_set(gp_widget *self, int val);

/**
 * @brief Toggles a checkbox value.
 *
 * @self A checkbox widget.
 */
void gp_widget_checkbox_toggle(gp_widget *self);

#endif /* GP_WIDGET_CHECKBOX_H__ */
