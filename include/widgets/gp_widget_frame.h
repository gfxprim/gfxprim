//SPDX-License-Identifier: LGPL-2.0-or-later

/*

   Copyright (c) 2014-2024 Cyril Hrubis <metan@ucw.cz>

 */

/**
 * @file gp_widget_frame.h
 * @brief A frame widget.
 *
 * @image html frame.png
 *
 * Frame widget JSON attributes
 * ----------------------------
 *
 * |   Attribute    |  Type  | Default | Description                                                |
 * |----------------|--------|---------|------------------------------------------------------------|
 * |   **title**    | string |         | A frame title.                                             |
 * |   **tattr**    | string | "bold"  | A title text attribute, parsed by gp_widget_tattr_parse(). |
 * |   **widget**   | object |         | A frame child widget.                                      |
 */

#ifndef GP_WIDGET_FRAME_H
#define GP_WIDGET_FRAME_H

/**
 * @brief Allocates and initializes a new frame widget.
 *
 * @param title A frame title, there is no title if NULL is passed.
 * @param tattr A text attribute monospace/bold/large etc.
 * @param child Frame widget child widget.
 *
 * @return A frame widget.
 */
gp_widget *gp_widget_frame_new(const char *title, gp_widget_tattr tattr, gp_widget *child);

/**
 * @brief Sets a new frame title.
 *
 * @param self A frame widget.
 * @param title New frame title.
 */
void gp_widget_frame_title_set(gp_widget *self, const char *title);

/**
 * @brief Puts a child widget into a frame widget.
 *
 * @param self A frame widget.
 * @param child A child widget.
 *
 * @return Returns previous frame widget child or NULL if frame had no child.
 */
gp_widget *gp_widget_frame_put(gp_widget *self, gp_widget *child);

#endif /* GP_WIDGET_FRAME_H */
