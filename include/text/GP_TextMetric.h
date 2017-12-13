/*****************************************************************************
 * This file is part of gfxprim library.                                     *
 *                                                                           *
 * Gfxprim is free software; you can redistribute it and/or                  *
 * modify it under the terms of the GNU Lesser General Public                *
 * License as published by the Free Software Foundation; either              *
 * version 2.1 of the License, or (at your option) any later version.        *
 *                                                                           *
 * Gfxprim is distributed in the hope that it will be useful,                *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of            *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU         *
 * Lesser General Public License for more details.                           *
 *                                                                           *
 * You should have received a copy of the GNU Lesser General Public          *
 * License along with gfxprim; if not, write to the Free Software            *
 * Foundation, Inc., 51 Franklin Street, Fifth Floor,                        *
 * Boston, MA  02110-1301  USA                                               *
 *                                                                           *
 * Copyright (C) 2009-2010 Jiri "BlueBear" Dluhos                            *
 *                         <jiri.bluebear.dluhos@gmail.com>                  *
 *                                                                           *
 * Copyright (C) 2009-2011 Cyril Hrubis <metan@ucw.cz>                       *
 *                                                                           *
 ****************************************************************************/

#ifndef TEXT_GP_TEXT_METRIC_H
#define TEXT_GP_TEXT_METRIC_H

#include <core/GP_Types.h>
#include <text/GP_TextStyle.h>

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
