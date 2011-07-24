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
 * Copyright (C) 2009-2011 Jiri "BlueBear" Dluhos                            *
 *                         <jiri.bluebear.dluhos@gmail.com>                  *
 *                                                                           *
 * Copyright (C) 2009-2011 Cyril Hrubis <metan@ucw.cz>                       *
 *                                                                           *
 *****************************************************************************/

#ifndef GP_TEXT_H
#define GP_TEXT_H

#include "core/GP_Context.h"

#include "GP_TextStyle.h"
#include "GP_TextMetric.h"

/* How the rendered text should be aligned.
 * For GP_Text(), the alignment is relative to the specified point:
 *
 *   - GP_ALIGN_LEFT draws the text to the left of the point,
 *   - GP_ALIGN_CENTER centers it at the point horizontally,
 *   - GP_ALIGN_RIGHT draws the text to the right of the point
 *   - GP_VALIGN_ABOVE (or TOP) draws the text above the point
 *   - GP_VALIGN_CENTER centers the text vertically at the point
 *   - GP_VALIGN_BASELINE places the text baseline at the point
 *   - GP_VALIGN_BELOW (or BOTTOM) draws the text below the point
 */
typedef enum GP_TextAlign {
	GP_ALIGN_LEFT = 0x01,
	GP_ALIGN_CENTER = 0x02,
	GP_ALIGN_RIGHT = 0x03,
	GP_VALIGN_ABOVE = 0x10,
	GP_VALIGN_TOP = GP_VALIGN_ABOVE,
	GP_VALIGN_CENTER = 0x20,
	GP_VALIGN_BASELINE = 0x30,
	GP_VALIGN_BELOW = 0x40,
	GP_VALIGN_BOTTOM = GP_VALIGN_BELOW,
} GP_TextAlign;

GP_RetCode GP_Text_Raw(GP_Context *context, const GP_TextStyle *style,
                       GP_Coord x, GP_Coord y, int align,
		       const char *str, GP_Pixel pixel);

GP_RetCode GP_Text(GP_Context *context, const GP_TextStyle *style,
                   GP_Coord x, GP_Coord y, int align,
		   const char *str, GP_Pixel pixel);

GP_RetCode GP_BoxCenteredText_Raw(GP_Context *context, const GP_TextStyle *style,
                                  GP_Coord x, GP_Coord y, GP_Size w, GP_Size h,
                                  const char *str, GP_Pixel pixel);

GP_RetCode GP_BoxCenteredText(GP_Context *context, const GP_TextStyle *style,
                              GP_Coord x, GP_Coord y, GP_Size w, GP_Size h,
                              const char *str, GP_Pixel pixel);

#endif /* GP_TEXT_H */
