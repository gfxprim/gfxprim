//SPDX-License-Identifier: LGPL-2.0-or-later

/*

   Copyright (c) 2014-2021 Cyril Hrubis <metan@ucw.cz>

 */

#ifndef GP_WIDGET_BOOL_H
#define GP_WIDGET_BOOL_H

struct gp_widget_bool {
	const char *label;
	int val;
	int type;
	char payload[];
};

/**
 * @brief Returns a class bool widget value.
 *
 * @self A bool class widget.
 * @return A boolean value.
 */
int gp_widget_bool_get(gp_widget *self);

/**
 * @brief Sets a class bool widget value.
 *
 * @self A bool class widget.
 * @val A boolean value.
 */
void gp_widget_bool_set(gp_widget *self, int val);

/**
 * @brief Toggles a class bool widget value.
 *
 * @self A bool class widget.
 */
static inline void gp_widget_bool_toggle(gp_widget *self)
{
	gp_widget_bool_set(self, !gp_widget_bool_get(self));
}

#endif /* GP_WIDGET_BOOL_H */
