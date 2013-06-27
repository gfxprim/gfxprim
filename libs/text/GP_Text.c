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

#include <stdarg.h>
#include "gfx/GP_Gfx.h"
#include "core/GP_FnPerBpp.h"
#include "core/GP_Debug.h"
#include "GP_Text.h"

GP_TextStyle GP_DefaultStyle = GP_DEFAULT_TEXT_STYLE;

static int do_align(GP_Coord *topleft_x, GP_Coord *topleft_y, int align,
                    GP_Coord x, GP_Coord y, const GP_TextStyle *style,
                    GP_Size width)
{
	int height = GP_TextHeight(style);

	switch (align & 0x0f) {
	case GP_ALIGN_LEFT:
		*topleft_x = x - width + 1;
		break;
	case GP_ALIGN_RIGHT:
		*topleft_x = x;
		break;
	case GP_ALIGN_CENTER:
		*topleft_x = x - width/2;
		break;
	default:
		GP_DEBUG(1, "ALIGN 0x%0x\n", align);
		return 1;
	}

	switch (align & 0x70) {
	case GP_VALIGN_ABOVE:
		*topleft_y = y - height + 1;
		break;
	case GP_VALIGN_CENTER:
		*topleft_y = y - height/2;
		break;
	case GP_VALIGN_BASELINE:
	//	*topleft_y = y - height + style->font->baseline;
		break;
	case GP_VALIGN_BELOW:
		*topleft_y = y;
		break;
	default:
		GP_DEBUG(1, "VALIGN 0x%0x\n", align);
		return 1;
	}

	return 0;
}

void GP_Text(GP_Context *context, const GP_TextStyle *style,
             GP_Coord x, GP_Coord y, int align,
	     GP_Pixel fg_color, GP_Pixel bg_color,
             const char *str)
{
	GP_CHECK_CONTEXT(context);

	if (str == NULL)
		return;

	if (style == NULL)
		style = &GP_DefaultStyle;

	GP_Coord topleft_x, topleft_y;
	GP_Size w = GP_TextWidth(style, str);

	GP_ASSERT(do_align(&topleft_x, &topleft_y, align, x, y, style, w) == 0,
	         "Invalid aligment flags");

	GP_Text_Raw(context, style, topleft_x, topleft_y,
	            align & GP_TEXT_NOBG, fg_color, bg_color, str);
}


GP_Size GP_Print(GP_Context *context, const GP_TextStyle *style,
                 GP_Coord x, GP_Coord y, int align,
                 GP_Pixel fg_color, GP_Pixel bg_color, const char *fmt, ...)
{
	va_list va, vac;
	int size;

	va_start(va, fmt);
	va_copy(vac, va);
	size = vsnprintf(NULL, 0, fmt, va);
	va_end(va);
	char buf[size+1];
	vsnprintf(buf, sizeof(buf), fmt, vac);
	va_end(vac);

	GP_Text(context, style, x, y, align, fg_color, bg_color, buf);

	return GP_TextWidth(style, buf);
}


void GP_TextClear(GP_Context *context, const GP_TextStyle *style,
                  GP_Coord x, GP_Coord y, int align,
		  GP_Pixel bg_color, GP_Size size)
{
	GP_Coord topleft_x, topleft_y;

	GP_ASSERT(do_align(&topleft_x, &topleft_y, align, x, y, style, size) == 0,
	         "Invalid aligment flags");

	GP_FillRectXYWH(context, topleft_x, topleft_y,
	                size, GP_TextHeight(style), bg_color);
}

void GP_TextClearStr(GP_Context *context, const GP_TextStyle *style,
                     GP_Coord x, GP_Coord y, int align,
		     GP_Pixel bg_color, const char *str)
{
	GP_TextClear(context, style, x, y, align,
	             bg_color, GP_TextWidth(style, str));
}
