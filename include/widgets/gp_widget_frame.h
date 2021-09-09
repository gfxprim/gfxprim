//SPDX-License-Identifier: LGPL-2.0-or-later

/*

   Copyright (c) 2014-2021 Cyril Hrubis <metan@ucw.cz>

 */

#ifndef GP_WIDGET_FRAME_H
#define GP_WIDGET_FRAME_H

struct gp_widget_frame {
	gp_widget *child;
	gp_widget_tattr tattr;
	int light_bg:1;
	char *title;
};

/**
 * @brief Allocates and initializes a new frame label.
 *
 * @label A frame label.
 * @bold If non-zero frame label test is bold.
 * @child Frame widget child widget.
 *
 * @return A frame widget.
 */
gp_widget *gp_widget_frame_new(const char *title, gp_widget_tattr tattr, gp_widget *child);

/**
 * @brief Sets a frame title.
 *
 * @label A frame label.
 * @title New title.
 */
void gp_widget_frame_title_set(gp_widget *self, const char *title);

/**
 * @brief Puts a child widget into a frame widget.
 *
 * @self A frame widget.
 * @child A child widget.
 *
 * @return Returns previous frame widget child or NULL if frame had no child.
 */
gp_widget *gp_widget_frame_put(gp_widget *self, gp_widget *child);

#endif /* GP_WIDGET_FRAME_H */
