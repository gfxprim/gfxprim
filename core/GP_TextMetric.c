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

static unsigned int SpaceWidth(const GP_TextStyle *style)
{
	//TODO: Does space change with pixel_yspace?
	return style->char_xspace * style->pixel_xmul;
}

unsigned int GP_TextWidth(const GP_TextStyle *style, const char *str)
{
	GP_CHECK(style != NULL && style->font != NULL && str != NULL);
	
	unsigned int x = 0;
	unsigned int space = SpaceWidth(style);
	unsigned int pixel_multiplier = style->pixel_xmul + style->pixel_xspace;

	// FIXME: This is not quite right - due to offsets, characters
	// can exceed their bounding box and then the reported width will be
	// shorter than expected.
	const char *p;
	for (p = str; *p; p++) {
		const GP_CharData *data = GP_GetCharData(style->font, *p);
		x += data->pre_offset * pixel_multiplier
			+ data->post_offset * pixel_multiplier
			+ space;
	}

	return x - space;
}

unsigned int GP_TextMaxWidth(const GP_TextStyle *style, unsigned int len)
{
	unsigned int space_width = SpaceWidth(style);
	//TODO: is style->font->char_width font character max width?
	unsigned int char_width  = style->font->char_width *
	                           (style->pixel_xmul + style->pixel_xspace);

	if (len == 0)
		return 0;

	return len * char_width + (len - 1) * space_width; 
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

