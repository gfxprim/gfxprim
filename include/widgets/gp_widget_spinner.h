//SPDX-License-Identifier: LGPL-2.0-or-later

/*

   Copyright (c) 2014-2024 Cyril Hrubis <metan@ucw.cz>

 */

/**
 * @file gp_widget_spinner.h
 * @brief A slider widget.
 *
 * Spinner is an integer class widget, after it's created it's changed by the
 * class functions such as gp_widget_int_val_set().
 */
#ifndef GP_WIDGET_SPINNER_H
#define GP_WIDGET_SPINNER_H

/**
 * @brief Allocate and initialize a spinner widget.
 *
 * @param min Spinner minimum.
 * @param max min Spinner maximum.
 * @param val Initial spinner value.
 *
 * @return A newly allocated and initialized spinner widget.
 */
gp_widget *gp_widget_spinner_new(int64_t min, int64_t max, int64_t val);

#endif /* GP_WIDGET_SPINNER_H */
