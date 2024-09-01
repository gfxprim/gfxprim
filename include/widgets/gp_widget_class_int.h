//SPDX-License-Identifier: LGPL-2.0-or-later

/*

   Copyright (c) 2014-2024 Cyril Hrubis <metan@ucw.cz>

 */

/**
 * @file gp_widget_class_int.h
 * @brief An integer class for widgets.
 *
 * Integer class implements common function to get/set value and limits for
 * integer type widget.
 */

#ifndef GP_WIDGET_CLASS_INT_H
#define GP_WIDGET_CLASS_INT_H

/**
 * @brief A widget int direction.
 */
enum gp_widget_int_flags {
	/** @brief Widget is horizontal. */
	GP_WIDGET_HORIZ = 0,
	/** @brief Widget is vertical. */
	GP_WIDGET_VERT = 1
};

typedef struct gp_widget_int {
	int64_t min;
	int64_t max;
	int64_t val;

	int alert:1;
	int dir:2;

	char payload[];
} gp_widget_class_int;

/**
 * @brief A macro to get a int class widget.
 *
 * @warning This is internal API do not use in applications!
 *
 * @warning The caller must make sure that the widget is bool class!
 */
#define GP_WIDGET_CLASS_INT(widget) ((gp_widget_class_int *)GP_WIDGET_PAYLOAD(widget))

/**
 * @brief A macro to get a int class widget payload.
 *
 * @warning This is internal API do not use in applications!
 *
 * @warning The caller must make sure that the widget is bool class!
 */
#define GP_WIDGET_CLASS_INT_PAYLOAD(widget) \
	((void*)(((gp_widget_class_int *)GP_WIDGET_PAYLOAD(widget))->payload))

/**
 * @brief Returns an integer class widget value.
 *
 * @param self An integer class widget.
 *
 * @return An integer class widget value.
 */
int64_t gp_widget_int_val_get(gp_widget *self);

/**
 * @brief Sets an integer class widget value and limits.
 *
 * @param self An integer class widget.
 * @param min New integer class widget minimum.
 * @param max New integer class widget maximum.
 * @param val New integer class widget value.
 */
void gp_widget_int_set(gp_widget *self, int64_t min, int64_t max, int64_t val);

/**
 * @brief Sets an integer class widget value.
 *
 * @param self An integer class widget.
 * @param val New integer class widget value.
 */
void gp_widget_int_val_set(gp_widget *self, int64_t val);

/**
 * @brief Sets an integer class widget maximum.
 *
 * @param self An integer class widget.
 * @param max New integer class widget maximum.
 */
void gp_widget_int_max_set(gp_widget *self, int64_t max);

/**
 * @brief Gets an integer class widget maximum.
 *
 * @param self An integer class widget.
 * @return An integer class widget maximum.
 */
int64_t gp_widget_int_max_get(gp_widget *self);

/**
 * @brief Sets an integer class widget minimum.
 *
 * @param self An integer class widget.
 * @param min New integer class widget minimum.
 */
void gp_widget_int_min_set(gp_widget *self, int64_t min);

/**
 * @brief Gets an integer class widget minimum.
 *
 * @param self An integer class widget.
 * @return An integer class widget minimun.
 */
int64_t gp_widget_int_min_get(gp_widget *self);

/**
 * @brief Sets an integer class widget limits.
 *
 * @param self An integer class widget.
 * @param min New integer class widget minimum.
 * @param max New integer class widget maximum.
 */
void gp_widget_int_set_range(gp_widget *self, int64_t min, int64_t max);

#endif /* GP_WIDGET_CLASS_INT_H */
