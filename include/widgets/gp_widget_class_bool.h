//SPDX-License-Identifier: LGPL-2.0-or-later

/*

   Copyright (c) 2014-2024 Cyril Hrubis <metan@ucw.cz>

 */

/**
 * @file gp_widget_class_bool.h
 * @brief A bollean widget class.
 *
 * Functions to change the value of a boolean class widget.
 */

#ifndef GP_WIDGET_CLASS_BOOL_H
#define GP_WIDGET_CLASS_BOOL_H

struct gp_widget_bool {
	const char *label;
	int val;
	int type;
	char payload[];
};

/**
 * @brief Returns a class bool widget value.
 *
 * @param self A bool class widget.
 * @return A boolean value.
 */
int gp_widget_bool_get(gp_widget *self);

/**
 * @brief Sets a class bool widget value.
 *
 * @param self A bool class widget.
 * @param val A boolean value.
 */
void gp_widget_bool_set(gp_widget *self, int val);

/**
 * @brief Toggles a class bool widget value.
 *
 * @param self A bool class widget.
 */
static inline void gp_widget_bool_toggle(gp_widget *self)
{
	gp_widget_bool_set(self, !gp_widget_bool_get(self));
}

#endif /* GP_WIDGET_CLASS_BOOL_H */
