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
 * Copyright (C) 2009-2010 Cyril Hrubis <metan@ucw.cz>                       *
 *                                                                           *
 *****************************************************************************/

#ifndef GP_COLORS_H
#define GP_COLORS_H

#include "GP_backend.h"

/* Indexes for basic colors obtained by GP_LoadBasicColors(). */
enum {
	GP_BLACK = 0,
	GP_RED,
	GP_GREEN,
	GP_BLUE,
	GP_YELLOW,
	GP_CYAN,
	GP_MAGENTA,
	GP_WHITE,

	GP_MID_RED,
	GP_MID_GREEN,
	GP_MID_BLUE,
	GP_MID_YELLOW,
	GP_MID_CYAN,
	GP_MID_MAGENTA,
	GP_MID_WHITE,
	GP_GRAY = GP_MID_WHITE,

	GP_DARK_RED,
	GP_DARK_GREEN,
	GP_DARK_BLUE,
	GP_DARK_YELLOW,
	GP_DARK_CYAN,
	GP_DARK_MAGENTA,
	GP_DARK_WHITE,
	GP_DARK_GRAY = GP_DARK_WHITE,

	GP_BASIC_COLOR_COUNT,
}
GP_BasicColor;

void GP_LoadBasicColors(GP_TARGET_TYPE *target, GP_COLOR_TYPE *colors);

#endif /* GP_COLORS_H */

