//SPDX-License-Identifier: LGPL-2.0-or-later

/*

   Copyright (c) 2014-2024 Cyril Hrubis <metan@ucw.cz>

 */

/**
 * @file gp_widget_slider.h
 * @brief A slider widget.
 *
 * Slider is an integer class widget, after it's created it's changed by the
 * class functions such as gp_widget_int_val_set().
 */
#ifndef GP_WIDGET_SLIDER_H
#define GP_WIDGET_SLIDER_H

/**
 * @brief Allocate and initialize a slider widget.
 *
 * @param min Slider minimum.
 * @param max Slider maximum.
 * @param val Initial slider value.
 * @param dir Slider direction.
 *
 * @return A slider widget.
 */
gp_widget *gp_widget_slider_new(int64_t min, int64_t max, int64_t val, int dir);

#endif /* GP_WIDGET_SLIDER_H */
