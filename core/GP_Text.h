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

#ifndef GP_TEXT_H
#define GP_TEXT_H

#include "GP_Context.h"

/* Where the text should be drawn relatively to the specified point */
typedef enum GP_TextAlign {
	GP_ALIGN_LEFT = 0x01,		/* to the left from the point */
	GP_ALIGN_CENTER = 0x02,		/* centered on the point */
	GP_ALIGN_RIGHT = 0x03,		/* to the right from the point */
	GP_VALIGN_ABOVE = 0x10,		/* above the point */
	GP_VALIGN_CENTER = 0x20,	/* centered on the point */
	GP_VALIGN_BASELINE = 0x30,	/* baseline is on the point */
	GP_VALIGN_BELOW = 0x40		/* below the point */
} GP_TextAlign;

GP_RetCode GP_Text(GP_Context *context, const GP_TextStyle *style,
                   int x, int y, int align, const char *str, GP_Pixel pixel);

GP_RetCode GP_TText(GP_Context *context, const GP_TextStyle *style,
                    int x, int y, int align, const char *str, GP_Pixel pixel);

#endif /* GP_TEXT_H */
