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

static unsigned int multiply_width(const GP_TextStyle *style, unsigned int w)
{
	return w * style->pixel_xmul + (w - 1) * style->pixel_xspace;
}

/*
 * Returns glyph advance.
 */
static unsigned int glyph_advance_x(const GP_TextStyle *style, int ch)
{
	const GP_GlyphBitmap *glyph = get_glyph(style, ch);

	return multiply_width(style, glyph->advance_x);
}

/*
 * Return maximal glyph advance for a character from str.
 */
static unsigned int max_glyph_advance_x(const GP_TextStyle *style,
                                        const char *str)
{
	unsigned int max = 0, i;

	for (i = 0; str[i] != '\0'; i++)
		max = GP_MAX(max, glyph_advance_x(style, str[i]));

	return max;
}

/*
 * Returns _SINGLE_ glyph size, not including the bearing_x and including space
 * occupied by the glyph bitmap if the bitmap width overflows glyph advance_x.
 */
static unsigned int glyph_width(const GP_TextStyle *style, int c)
{
	unsigned int size, advance;

	const GP_GlyphBitmap *glyph = get_glyph(style, c);

	advance = multiply_width(style, glyph->advance_x - glyph->bearing_x);
	size    = multiply_width(style, glyph->width);

	return (size > advance) ? size : advance;
}

/*
 * Returns size occupied by the last glyph. Here we take advance_x into account.
 */
static unsigned int last_glyph_width(const GP_TextStyle *style, int c)
{
	unsigned int size, advance;

	const GP_GlyphBitmap *glyph = get_glyph(style, c);

	advance = multiply_width(style, glyph->advance_x);
	size    = multiply_width(style, glyph->width + glyph->bearing_x);

	return (size > advance) ? size : advance;
}

/*
 * Returns first glyph width, that is and advance minus the bearing_x.
 */
static unsigned int first_glyph_width(const GP_TextStyle *style, int c)
{
	const GP_GlyphBitmap *glyph = get_glyph(style, c);

	return multiply_width(style, glyph->advance_x - glyph->bearing_x);
}

static const GP_TextStyle *assert_style(const GP_TextStyle *style)
{
	if (style == NULL)
		style = &GP_DefaultStyle;

	GP_ASSERT(style->font != NULL);

	return style;
}

/*
 * Returns text width.
 *
 * There are two problems with text width it's start and it's end.
 *
 * First letter may have bearing_x negative, making it overflow out of the
 * bouding box and even in case it's possitive the returned size would be
 * slightly bigger. This one is easy to fix.
 *
 * The end of the string is problematic too, some of the glyphs may have
 * advance smaller than sum of the bitmap width and bearing_x that way we would
 * return slightly less than is the actuall size of the text bouding box. The
 * other problem is that the advance is usually grater than the glyph space (as
 * the exact size of the glyph itself without advance before and space after is
 * not known), so we likely return some more pixels than is needed, but that's
 * the joy of life.
 */
unsigned int GP_TextWidth(const GP_TextStyle *style, const char *str)
{
	unsigned int i, len;

	style = assert_style(style);

	if (str == NULL || str[0] == '\0')
		return 0;

	/* special case, one letter */
	if (str[1] == '\0')
		return glyph_width(style, str[0]);

	/* first letter */
	len = first_glyph_width(style, str[0]) + style->char_xspace;

	/* middle letters */
	for (i = 1; str[i+1] != '\0'; i++)
		len += glyph_advance_x(style, str[i]) + style->char_xspace;

	/* last letter */
	len += last_glyph_width(style, str[i]);

	return len;
}

/*
 * Return max advance now. We will do something better maybe.
 */
GP_Size GP_TextMaxWidth(const GP_TextStyle *style, unsigned int len)
{
	style = assert_style(style);

	if (len == 0)
		return 0;

	return multiply_width(style, len * style->font->max_glyph_advance) +
	       (len - 1) * style->char_xspace;
}

/*
 * Here too.
 */
GP_Size GP_TextMaxStrWidth(const GP_TextStyle *style, const char *str,
                           unsigned int len)
{
	style = assert_style(style);

	if (len == 0 || str == NULL)
		return 0;

	return len * max_glyph_advance_x(style, str) +
	       (len - 1) * style->char_xspace;
}

/* Ascend, Descend, Height -- far easier */

static unsigned int multiply_height(const GP_TextStyle *style, unsigned int h)
{
	return h * style->pixel_ymul + (h - 1) * style->pixel_yspace;
}

GP_Size GP_TextHeight(const GP_TextStyle *style)
{
	style = assert_style(style);

	unsigned int height = style->font->ascend + style->font->descend;

	return multiply_height(style, height);
}

GP_Size GP_TextAscent(const GP_TextStyle *style)
{
	style = assert_style(style);

	return multiply_height(style, style->font->ascend);
}

GP_Size GP_TextDescent(const GP_TextStyle *style)
{
	style = assert_style(style);

	return multiply_height(style, style->font->descend);
}
