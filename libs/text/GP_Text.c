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

#include "algo/Text.algo.h"
#include "gfx/GP_Gfx.h"
#include "core/GP_FnPerBpp.h"
#include "core/GP_DefFnPerBpp.h"
#include "GP_Text.h"

static GP_TextStyle DefaultStyle = GP_DEFAULT_TEXT_STYLE;

/* Generate drawing functions for various bit depths. */
GP_DEF_FILL_FN_PER_BPP(GP_Text, DEF_TEXT_FN)

DEF_TEXT_FN(GP_TText_internal, GP_Context *, GP_Pixel, GP_THLine)

GP_RetCode GP_Text(GP_Context *context, const GP_TextStyle *style,
	int x, int y, int align, const char *str, GP_Pixel pixel)
{
	GP_CHECK_CONTEXT(context);
	GP_CHECK_TEXT_STYLE(style);

	if (str == NULL)
		return GP_ENULLPTR;


	if (style == NULL)
		style = &DefaultStyle;

	int width = GP_TextWidth(style, str);
	int height = GP_TextHeight(style);

	int topleft_x, topleft_y;
	switch (align & 0x0f) {
		case GP_ALIGN_LEFT:
			topleft_x = x - width + 1;
			break;
		case GP_ALIGN_RIGHT:
			topleft_x = x;
			break;
		case GP_ALIGN_CENTER:
			topleft_x = x - width/2;
			break;
		default:
			return GP_EINVAL;
	}
	switch (align & 0xf0) {
		case GP_VALIGN_ABOVE:
			topleft_y = y - height + 1;
			break;
		case GP_VALIGN_CENTER:
			topleft_y = y - height/2;
			break;
		case GP_VALIGN_BASELINE:
			topleft_y = y - height + style->font->baseline;
			break;
		case GP_VALIGN_BELOW:
			topleft_y = y;
			break;
		default:
			return GP_EINVAL;
	}

	GP_FN_PER_BPP_CONTEXT(GP_Text, context, context,
	              style, topleft_x, topleft_y, str, pixel);

	return GP_ESUCCESS;
}

GP_RetCode GP_TText(GP_Context *context, const GP_TextStyle *style,
	int x, int y, int align, const char *str, GP_Pixel pixel)
{
	GP_CHECK_CONTEXT(context);
	GP_CHECK_TEXT_STYLE(style);
	
	if (str == NULL)
		return GP_ENULLPTR;

	if (style == NULL)
		style = &DefaultStyle;

	int width = GP_TextWidth(style, str);
	int height = GP_TextHeight(style);

	int topleft_x, topleft_y;
	switch (align & 0x0f) {
		case GP_ALIGN_LEFT:
			topleft_x = x - width;
			break;
		case GP_ALIGN_RIGHT:
			topleft_x = x;
			break;
		case GP_ALIGN_CENTER:
			topleft_x = x - width/2;
			break;
		default:
			return GP_EINVAL;
	}
	switch (align & 0xf0) {
		case GP_VALIGN_ABOVE:
			topleft_y = y - height;
			break;
		case GP_VALIGN_CENTER:
			topleft_y = y - height/2;
			break;
		case GP_VALIGN_BASELINE:
			topleft_y = y - height + style->font->baseline;
			break;
		case GP_VALIGN_BELOW:
			topleft_y = y;
			break;
		default:
			return GP_EINVAL;
	}

	GP_TText_internal(context, style, topleft_x, topleft_y, str, pixel);
	return GP_ESUCCESS;
}

GP_RetCode GP_BoxCenteredText(GP_Context *context, const GP_TextStyle *style,
	int x, int y, int w, int h, const char *str, GP_Pixel pixel)
{
	GP_CHECK_CONTEXT(context);
	GP_CHECK_TEXT_STYLE(style);

	if (str == NULL)
		return GP_ENULLPTR;

	if (style == NULL)
		style = &DefaultStyle;

	const int mid_x = x + w/2;
	const int mid_y = y + h/2;
	const int font_ascent = GP_TextAscent(style);

	return GP_Text(context, style, mid_x,
		mid_y + font_ascent/2,
		GP_ALIGN_CENTER | GP_VALIGN_BASELINE,
		str, pixel);
}

GP_RetCode GP_TBoxCenteredText(GP_Context *context, const GP_TextStyle *style,
	int x, int y, int w, int h, const char *str, GP_Pixel pixel)
{
	GP_CHECK_CONTEXT(context);
	GP_CHECK_TEXT_STYLE(style);

	if (str == NULL)
		return GP_ENULLPTR;

	if (style == NULL)
		style = &DefaultStyle;

	const int mid_x = x + w/2;
	const int mid_y = y + h/2;
	const int font_ascent = GP_TextAscent(style);

	return GP_TText(context, style, mid_x,
		mid_y + font_ascent/2,
		GP_ALIGN_CENTER | GP_VALIGN_BASELINE,
		str, pixel);
}
