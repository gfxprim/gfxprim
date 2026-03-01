//SPDX-License-Identifier: LGPL-2.0-or-later

/*

   Copyright (c) 2014-2026 Cyril Hrubis <metan@ucw.cz>

 */

/**
 * @file gp_widget_activity.h
 * @brief An activity widget.
 *
 * Activity widget
 * ---------------
 *
 * Activity widget is an animation which shows that application doing
 * something. Unlike progress bar there is no measurement of progress of the
 * work being done.
 *
 * Activity widget JSON attributes
 * -------------------------------
 *
 * |   Attribute    |  Type  | Default | Description        |
 * |----------------|--------|---------|--------------------|
 * |   **label**    | string |         | An activity label. |
 */

#ifndef GP_WIDGET_ACTIVITY_H
#define GP_WIDGET_ACTIVITY_H

/**
 * @brief Allocates and initializes a new activity widget.
 *
 * @param label An activity label.
 *
 * @return An activity widget.
 */
gp_widget *gp_widget_activity_new(const char *label);

/**
 * @brief Starts the activity widget animation.
 */
void gp_widget_activity_start(gp_widget *self);

/**
 * @brief Stops the activity widget animation.
 */
void gp_widget_activity_stop(gp_widget *self);

/**
 * @brief Returns true if activity is running.
 *
 * @return True if activity is running.
 */
bool gp_widget_activity_running(gp_widget *self);

/**
 * @brief Gets an activity label.
 *
 * @param self An activity widget.
 *
 * @return An activity label or NULL if there is none.
 */
const char *gp_widget_activity_label_get(gp_widget *self);

#endif /* GP_WIDGET_ACTIVITY_H */
