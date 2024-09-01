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

typedef struct gp_widget_bool {
	/** @brief A boolean value of the widget. */
	bool val;
	/** @brief Points to the widget private data. */
	char payload[];
} gp_widget_class_bool;

/**
 * @brief A macro to get a bool class widget.
 *
 * @warning This is internal API do not use in applications!
 *
 * @warning The caller must make sure that the widget is bool class!
 */
#define GP_WIDGET_CLASS_BOOL(widget) ((gp_widget_class_bool *)GP_WIDGET_PAYLOAD(widget))

/**
 * @brief A macro to get a bool class widget payload.
 *
 * @warning This is internal API do not use in applications!
 *
 * @warning The caller must make sure that the widget is bool class!
 */
#define GP_WIDGET_CLASS_BOOL_PAYLOAD(widget) \
	((void*)(((gp_widget_class_bool *)GP_WIDGET_PAYLOAD(widget))->payload))

/**
 * @brief Returns a class bool widget value.
 *
 * @param self A bool class widget.
 * @return A boolean value.
 */
bool gp_widget_bool_get(gp_widget *self);

/**
 * @brief Sets a class bool widget value.
 *
 * @param self A bool class widget.
 * @param val A boolean value.
 */
void gp_widget_bool_set(gp_widget *self, bool val);

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
