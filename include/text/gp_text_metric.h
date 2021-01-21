// SPDX-License-Identifier: GPL-2.1-or-later
/*
 * Copyright (C) 2009-2010 Jiri "BlueBear" Dluhos
 *                         <jiri.bluebear.dluhos@gmail.com>
 *
 * Copyright (C) 2009-2011 Cyril Hrubis <metan@ucw.cz>
 */

#ifndef TEXT_GP_TEXT_METRIC_H
#define TEXT_GP_TEXT_METRIC_H

#include <core/gp_types.h>
#include <text/gp_text_style.h>

/*
 * Calculates the width of the string drawn in the given style, in pixels.
 */
gp_size gp_text_width_len(const gp_text_style *style,
                          const char *str, size_t len);

gp_size gp_text_width(const gp_text_style *style, const char *str);

/*
 * Maximal text width for string with len characters.
 */
gp_size gp_text_max_width(const gp_text_style *style, unsigned int len);

/*
 * Returns average width for len characters.
 */
gp_size gp_text_avg_width(const gp_text_style *style, unsigned int len);

/*
 * Returns maximal width for text written with len characters from str.
 */
gp_size gp_text_max_width_chars(const gp_text_style *style, const char *chars,
                                unsigned int len);

/*
 * Returns maximal text height, in pixels.
 */
gp_size gp_text_height(const gp_text_style *style);

/*
 * Returns the ascent (height from the baseline to the top of characters),
 * for the given text style. (Result is in pixels.)
 */
gp_size gp_text_ascent(const gp_text_style *style);

/*
 * Returns the descent (height from the baseline to the bottom of characters),
 * for the given text style. (Result is in pixels.)
 */
gp_size gp_text_descent(const gp_text_style *style);

#endif /* TEXT_GP_TEXT_METRIC_H */
