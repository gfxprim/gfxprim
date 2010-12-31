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

#include "GP.h"

const uint8_t *GP_GetCharData(const GP_TextStyle *style, char c)
{
	GP_CHECK(style != NULL && style->font != NULL);

	int bytes_per_char = 2 + style->font->bytes_per_line * style->font->height;

	const uint8_t *char_data = style->font->data + ((int) c - 0x20) * bytes_per_char;

	return char_data;
}

static unsigned int GP_CharWidth(const GP_TextStyle *style, char c)
{
	const uint8_t *char_data = GP_GetCharData(style, c);

	/* The first byte of the font data is width in pixels. */
	const uint8_t char_width = *char_data;

	return char_width * (style->pixel_xmul + style->pixel_xspace);
}

unsigned int GP_TextWidth(const GP_TextStyle *style, const char *str)
{
	GP_CHECK(style != NULL && style->font != NULL && str != NULL);
	
	unsigned int width = 0;
	//TODO: Does space change with pixel_yspace?
	unsigned int space = style->font->hspace * style->pixel_xmul;

	for (; *str; str++)
		width += GP_CharWidth(style, *str) + space; 

	if (width == 0)
		return 0;

	return width - space;
}

unsigned int GP_TextHeight(const GP_TextStyle *style)
{
	GP_CHECK(style != NULL && style->font != NULL);

	return style->font->height * style->pixel_ymul +
	       (style->font->height - 1) * style->pixel_yspace;
}

unsigned int GP_TextAscent(const GP_TextStyle *style)
{
	GP_CHECK(style != NULL && style->font != NULL);

	unsigned int h = style->font->height - style->font->baseline;
	return h * style->pixel_ymul + (h - 1) * style->pixel_yspace;
}

unsigned int GP_TextDescent(const GP_TextStyle *style)
{
	GP_CHECK(style != NULL && style->font != NULL);

	unsigned int h = style->font->baseline;
	return h * style->pixel_ymul + (h - 1) * style->pixel_yspace;
}

