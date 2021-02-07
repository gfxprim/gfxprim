//SPDX-License-Identifier: LGPL-2.0-or-later

/*

   Copyright (c) 2014-2020 Cyril Hrubis <metan@ucw.cz>

 */

#ifndef GP_WIDGET_SLIDER_H
#define GP_WIDGET_SLIDER_H

/**
 * @brief Allocate and initialize a slider widget.
 *
 * @min Slider minimum.
 * @max Slider maximum.
 * @val Initial slider value.
 * @dir Slider direction.
 * @on_event Slider event handler.
 * @priv User private pointer.
 *
 * @return A slider widget.
 */
gp_widget *gp_widget_slider_new(int min, int max, int val, int dir,
                                int (*on_event)(gp_widget_event *ev),
                                void *priv);

/**
 * @brief Sets a slider widget value.
 *
 * @self A slider widget.
 * @val New slider value.
 */
void gp_widget_slider_set(gp_widget *self, int val);

/**
 * @brief Returns slider value.
 *
 * @self A slider widget.
 * @return Slider value.
 */
int gp_widget_slider_get(gp_widget *self);

#endif /* GP_WIDGET_SLIDER_H */
