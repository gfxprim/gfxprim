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

#include "core/GP_Types.h"
#include "GP_TextStyle.h"

/*
 * Calculates the width of the string drawn in the given style, in pixels.
 */
GP_Size GP_TextLenWidth(const GP_TextStyle *style,
                        const char *str, size_t len);

GP_Size GP_TextWidth(const GP_TextStyle *style, const char *str);

/*
 * Maximal text width for string with len characters.
 */
GP_Size GP_TextMaxWidth(const GP_TextStyle *style, unsigned int len);

/*
 * Returns maximal width for text written with len characters from str.
 */
GP_Size GP_TextMaxStrWidth(const GP_TextStyle *style, const char *str,
                           unsigned int len);

/*
 * Returns maximal text height, in pixels.
 */
GP_Size GP_TextHeight(const GP_TextStyle *style);

/*
 * Returns the ascent (height from the baseline to the top of characters),
 * for the given text style. (Result is in pixels.)
 */
GP_Size GP_TextAscent(const GP_TextStyle *style);

/*
 * Returns the descent (height from the baseline to the bottom of characters),
 * for the given text style. (Result is in pixels.)
 */
GP_Size GP_TextDescent(const GP_TextStyle *style);

#endif /* TEXT_GP_TEXT_METRIC_H */
