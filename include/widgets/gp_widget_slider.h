//SPDX-License-Identifier: LGPL-2.0-or-later

/*

   Copyright (c) 2014-2023 Cyril Hrubis <metan@ucw.cz>

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
 *
 * @return A slider widget.
 */
gp_widget *gp_widget_slider_new(int64_t min, int64_t max, int64_t val, int dir);

#endif /* GP_WIDGET_SLIDER_H */
