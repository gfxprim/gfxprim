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

#include "core/GP_Common.h"
#include "GP_TextMetric.h"

extern GP_TextStyle GP_DefaultStyle;

static unsigned int SpaceWidth(const GP_TextStyle *style)
{
	//TODO: Does space change with pixel_yspace?
	return style->char_xspace * style->pixel_xmul;
}

// FIXME: This is not quite right - due to offsets, characters
// can exceed their bounding box and then the reported width will be
// shorter than expected.
static unsigned int CharWidth(const GP_TextStyle *style, char ch)
{
	unsigned int pixel_multiplier = style->pixel_xmul + style->pixel_xspace;
	const GP_CharData *data = GP_GetCharData(style->font, ch);

	if (data == NULL)
		data = GP_GetCharData(style->font, ' ');

	return data->pre_offset * pixel_multiplier
	       + data->post_offset * pixel_multiplier;
}

static unsigned int MaxCharsWidth(const GP_TextStyle *style, const char *str)
{
	unsigned int max = 0, i;

	for (i = 0; str[i] != '\0'; i++)
		max = GP_MAX(max, CharWidth(style, str[i]));

	return max;
}

unsigned int GP_TextWidth(const GP_TextStyle *style, const char *str)
{
	GP_CHECK(str, "NULL string specified");
	
	unsigned int i, len = 0;
	unsigned int space;

	if (style == NULL)
		style = &GP_DefaultStyle;

	space = SpaceWidth(style);

	if (str[0] == '\0')
		return 0;

	for (i = 0; str[i] != '\0'; i++)
		len += CharWidth(style, str[i]);

	return len + (i - 1) * space;
}

unsigned int GP_TextMaxWidth(const GP_TextStyle *style, unsigned int len)
{
	unsigned int space_width = SpaceWidth(style);
	unsigned int char_width;
	
	if (style == NULL)
		style = &GP_DefaultStyle;

	char_width = style->font->max_bounding_width
		     * (style->pixel_xmul + style->pixel_xspace);

	if (len == 0)
		return 0;

	return len * char_width + (len - 1) * space_width; 
}

unsigned int GP_TextMaxStrWidth(const GP_TextStyle *style, const char *str,
                                unsigned int len)
{
	GP_CHECK(str, "NULL string specified");

	unsigned int space_width;
	unsigned int char_width;
	
	if (style == NULL)
		style = &GP_DefaultStyle;
	
	space_width = SpaceWidth(style);
	
	if (len == 0)
		return 0;

	char_width = MaxCharsWidth(style, str);

	return len * char_width + (len - 1) * space_width;
}

unsigned int GP_TextHeight(const GP_TextStyle *style)
{
	if (style == NULL)
		style = &GP_DefaultStyle;

	return style->font->height * style->pixel_ymul +
	       (style->font->height - 1) * style->pixel_yspace;
}

unsigned int GP_TextAscent(const GP_TextStyle *style)
{
	unsigned int h;

	if (style == NULL)
		style = &GP_DefaultStyle;

	h = style->font->height - style->font->baseline;
	
	return h * style->pixel_ymul + (h - 1) * style->pixel_yspace;
}

unsigned int GP_TextDescent(const GP_TextStyle *style)
{
	unsigned int h;
	
	if (style == NULL)
		style = &GP_DefaultStyle;

	h = style->font->baseline;
	
	return h * style->pixel_ymul + (h - 1) * style->pixel_yspace;
}

