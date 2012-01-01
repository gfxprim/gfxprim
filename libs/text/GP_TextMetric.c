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
 * Copyright (C) 2009-2012 Cyril Hrubis <metan@ucw.cz>                       *
 *                                                                           *
 *****************************************************************************/

#include "core/GP_Common.h"
#include "GP_TextMetric.h"

extern GP_TextStyle GP_DefaultStyle;

static const GP_GlyphBitmap *get_glyph(const GP_TextStyle *style, int c)
{
	const GP_GlyphBitmap *glyph = GP_GetGlyphBitmap(style->font, c);

	if (glyph == NULL)
		glyph = GP_GetGlyphBitmap(style->font, ' ');
	
	return glyph;
}

static unsigned int get_pixel_multiplier(const GP_TextStyle *style)
{
	return style->pixel_xmul + style->pixel_xspace;
}

/*
 * Returns glyph width from the basepoint to the end of the glyph bitmap.
 */
static unsigned int glyph_width(const GP_TextStyle *style, int c)
{
	unsigned int pixel_multiplier = get_pixel_multiplier(style); 
	const GP_GlyphBitmap *glyph = get_glyph(style, c);

	return (glyph->width + glyph->bearing_x) * pixel_multiplier;
}

/*
 * Returns size of the glyph but doesn't count bearing.
 *
 * Note that the bearing may be negative (typical case is letter 'j' or most of
 * the italic glyphs). So in order not to draw out of the text bouding box,
 * first glyph ignores bearing_x and draws beginning of the glyph bitmap on the
 * starting basepoint.
 *
 */
static unsigned int first_glyph_width(const GP_TextStyle *style, int c)
{
	unsigned int pixel_multiplier = get_pixel_multiplier(style);
	const GP_GlyphBitmap *glyph = get_glyph(style, c);

	return glyph->width * pixel_multiplier;
}

/*
 * Returns space that is added after the glyph.
 *
 * Eg. the advance minus glyph width.
 *
 * TODO: kerning
 */
static unsigned int glyph_space(const GP_TextStyle *style, char ch)
{
	unsigned int pixel_multiplier = get_pixel_multiplier(style);
	const GP_GlyphBitmap *glyph = get_glyph(style, ch);
	
	return (glyph->advance_x - glyph->width - glyph->bearing_x)
	       * pixel_multiplier + style->char_xspace;
}

/*
 * Returns maximal character width for a given string.
 */
static unsigned int max_glyph_width(const GP_TextStyle *style, const char *str)
{
	unsigned int max = 0, i;

	for (i = 0; str[i] != '\0'; i++)
		max = GP_MAX(max, glyph_width(style, str[i]));

	return max;
}

/*
 * Return maximal character space after an glyph.
 */
static unsigned int max_glyph_space(const GP_TextStyle *style, const char *str)
{
	unsigned int max = 0, i;

	for (i = 0; str[i] != '\0'; i++)
		max = GP_MAX(max, glyph_space(style, str[i]));

	return max;
}

unsigned int GP_TextWidth(const GP_TextStyle *style, const char *str)
{
	unsigned int i, len = 0;

	if (style == NULL)
		style = &GP_DefaultStyle;

	if (str == NULL || str[0] == '\0')
		return 0;

	len += first_glyph_width(style, str[0]);
	
	if (str[1] == '\0')
		return len;

	len += glyph_space(style, str[0]);

	for (i = 1; str[i] != '\0'; i++) {
		len += glyph_width(style, str[i]);

		if (str[i+1] != '\0')
			len += glyph_space(style, str[i]);
	}

	return len;
}

GP_Size GP_TextMaxWidth(const GP_TextStyle *style, unsigned int len)
{
	unsigned int glyph_width;
	//TODO: !
	unsigned int space_width = style->char_xspace;

	if (style == NULL)
		style = &GP_DefaultStyle;

	glyph_width = style->font->max_glyph_width
	              * (style->pixel_xmul + style->pixel_xspace);

	if (len == 0)
		return 0;

	return len * glyph_width + (len - 1) * space_width; 
}

GP_Size GP_TextMaxStrWidth(const GP_TextStyle *style, const char *str,
                           unsigned int len)
{
	unsigned int space_width;
	unsigned int glyph_width;
	
	if (style == NULL)
		style = &GP_DefaultStyle;
	
	space_width = max_glyph_space(style, str);
	
	if (len == 0 || str == NULL)
		return 0;

	glyph_width = max_glyph_width(style, str);

	return len * glyph_width + (len - 1) * space_width;
}

GP_Size GP_TextHeight(const GP_TextStyle *style)
{
	unsigned int h;
	
	if (style == NULL)
		style = &GP_DefaultStyle;

	h = style->font->ascend + style->font->descend;

	return h * style->pixel_ymul +
	       (h - 1) * style->pixel_yspace;
}

GP_Size GP_TextAscent(const GP_TextStyle *style)
{
	unsigned int h;

	if (style == NULL)
		style = &GP_DefaultStyle;

	h = style->font->ascend;
	
	return h * style->pixel_ymul + (h - 1) * style->pixel_yspace;
}

GP_Size GP_TextDescent(const GP_TextStyle *style)
{
	unsigned int h;
	
	if (style == NULL)
		style = &GP_DefaultStyle;

	h = style->font->descend;
	
	return h * style->pixel_ymul + (h - 1) * style->pixel_yspace;
}
