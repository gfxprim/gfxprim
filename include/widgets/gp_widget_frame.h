//SPDX-License-Identifier: LGPL-2.0-or-later

/*

   Copyright (c) 2014-2020 Cyril Hrubis <metan@ucw.cz>

 */

#ifndef GP_WIDGET_FRAME_H__
#define GP_WIDGET_FRAME_H__

struct gp_widget_frame {
	gp_widget *child;
	uint8_t has_label:1;
	uint8_t bold:1;
	char label[];
};

/**
 * @brief Allocates and initializes a new frame label.
 *
 * @label: A frame label.
 * @bold: If non-zero frame label test is bold.
 * @child: Frame widget child widget.
 */
gp_widget *gp_widget_frame_new(const char *label, int bold, gp_widget *child);

/**
 * @brief Puts a child widget into a frame widget.
 *
 * @self A frame widget.
 * @child A child widget.
 *
 * @return Returns previous frame widget child or NULL if frame had no child.
 */
gp_widget *gp_widget_frame_put(gp_widget *self, gp_widget *child);

#endif /* GP_WIDGET_FRAME_H__ */
