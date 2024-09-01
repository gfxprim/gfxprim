//SPDX-License-Identifier: LGPL-2.0-or-later

/*

   Copyright (c) 2014-2020 Cyril Hrubis <metan@ucw.cz>

 */

#ifndef GP_WIDGET_SCROLL_AREA_H
#define GP_WIDGET_SCROLL_AREA_H

/**
 * @brief Allocate and initialize a scroll area widget.
 *
 * @min_w Minimal width.
 * @min_h Minimal height.
 * @child A child widget.
 *
 * @return A scroll area widget.
 */
gp_widget *gp_widget_scroll_area_new(gp_size min_w, gp_size min_h, gp_widget *child);

/**
 * @brief Move a scroll area.
 *
 * @x_off X offset.
 * @y_off Y offset.
 *
 * @return Returns non-zero if area was moved zero otherwise.
 */
int gp_widget_scroll_area_move(gp_widget *self, gp_coord x_off, gp_coord y_off);

/**
 * @brief Puts a child widgets into a scroll area widget.
 *
 * @self A scroll area widget.
 * @child A child widget.
 *
 * @return Returns previous scroll area widget child or NULL if frame had no child.
 */
gp_widget *gp_widget_scroll_area_put(gp_widget *self, gp_widget *child);

#endif /* GP_WIDGET_SCROLL_AREA_H */
