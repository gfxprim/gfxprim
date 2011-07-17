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
 *****************************************************************************/

#ifndef GP_COLOR_H
#define GP_COLOR_H

#include "GP_Context.h"
#include "GP_Pixel.h"

typedef enum GP_ColorName {
	GP_COL_BLACK,
	GP_COL_RED,
	GP_COL_GREEN,
	GP_COL_BLUE,
	GP_COL_YELLOW,
	GP_COL_BROWN,
	GP_COL_ORANGE,
	GP_COL_GRAY_DARK,
	GP_COL_GRAY_LIGHT,
	GP_COL_PURPLE,
	GP_COL_WHITE,
	GP_COL_MAX,
} GP_ColorName;

GP_Pixel GP_ColorNameToPixel(GP_Context *context, GP_ColorName name); 

#endif /* GP_COLOR_H */
