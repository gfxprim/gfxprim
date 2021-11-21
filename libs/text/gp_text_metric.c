// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2009-2010 Jiri "BlueBear" Dluhos
 *                         <jiri.bluebear.dluhos@gmail.com>
 *
 * Copyright (C) 2009-2021 Cyril Hrubis <metan@ucw.cz>
 */

#include "core/gp_common.h"
#include <text/gp_text_metric.h>

extern gp_text_style gp_default_style;

static const gp_glyph *get_glyph(const gp_text_style *style, int c)
{
	const gp_glyph *glyph = gp_get_glyph(style->font, c);

	if (glyph == NULL)
		glyph = gp_get_glyph(style->font, ' ');

	return glyph;
}

static unsigned int multiply_width(const gp_text_style *style, unsigned int w)
{
	return w * style->pixel_xmul + (w - 1) * style->pixel_xspace;
}

/*
 * Returns glyph advance.
 */
static unsigned int glyph_advance_x(const gp_text_style *style, int ch)
{
	const gp_glyph *glyph = get_glyph(style, ch);

	return multiply_width(style, glyph->advance_x);
}

/*
 * Returns glyph bearing.
 */
static unsigned int glyph_bearing_x(const gp_text_style *style, int ch)
{
	const gp_glyph *glyph = get_glyph(style, ch);

	return multiply_width(style, glyph->bearing_x);
}

/*
 * Return maximal glyph advance for a character from str.
 */
static unsigned int max_glyph_advance_x(const gp_text_style *style,
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
static unsigned int glyph_width(const gp_text_style *style, int c)
{
	unsigned int size, advance;

	const gp_glyph *glyph = get_glyph(style, c);

	advance = multiply_width(style, glyph->advance_x - glyph->bearing_x);
	size    = multiply_width(style, glyph->width);

	return (size > advance) ? size : advance;
}

/*
 * Returns size occupied by the last glyph.
 *
 * The end of the string is problematic too, some of the glyphs may have
 * advance smaller than sum of the bitmap width and bearing_x that way we would
 * return slightly less than is the actuall size of the text bouding box. The
 * other problem is that the advance is usually grater than the glyph space (as
 * the exact size of the glyph itself without advance before and space after is
 * not known), so we likely return some more pixels than is needed.
 */
static unsigned int last_glyph_width(const gp_text_style *style, int c)
{
	unsigned int size, advance;

	const gp_glyph *glyph = get_glyph(style, c);

	advance = multiply_width(style, glyph->advance_x);
	size = multiply_width(style, glyph->width + glyph->bearing_x);

	return (size > advance) ? size : advance;
}

/*
 * Returns first glyph width, that is and advance minus the bearing_x.
 *
 * First letter may have bearing_x negative, making it overflow out of the
 * bouding box and even in case it's possitive the returned size would be
 * slightly bigger.
 */
static unsigned int first_glyph_width(const gp_text_style *style, int c)
{
	const gp_glyph *glyph = get_glyph(style, c);

	return multiply_width(style, glyph->advance_x - glyph->bearing_x);
}

static const gp_text_style *assert_style(const gp_text_style *style)
{
	if (style == NULL)
		style = &gp_default_style;

	GP_ASSERT(style->font != NULL);

	return style;
}

gp_size gp_text_width_len(const gp_text_style *style, enum gp_text_len_type type,
                          const char *str, size_t len)
{
	unsigned int i, ret;

	style = assert_style(style);

	if (!str || !str[0] || !len)
		return 0;

	/* special case, single letter */
	if (str[1] == '\0' || len == 1)
		return glyph_width(style, str[0]);

	/* first letter */
	ret = first_glyph_width(style, str[0]) + style->char_xspace;

	/* middle letters */
	for (i = 1; i + 1 < len && str[i+1]; i++) {
		ret += glyph_bearing_x(style, str[i]);
		ret += glyph_advance_x(style, str[i]);
		ret += style->char_xspace;
	}

	/* last letter */
	switch (type) {
	case GP_TEXT_LEN_BBOX:
		ret += last_glyph_width(style, str[i]);
	break;
	case GP_TEXT_LEN_ADVANCE:
		ret += glyph_advance_x(style, str[i]) + style->char_xspace;

		if (str[i+1])
			ret += glyph_bearing_x(style, str[i+1]);
	break;
	}

	return ret;
}

gp_size gp_text_width(const gp_text_style *style, enum gp_text_len_type type,const char *str)
{
	return gp_text_width_len(style, type, str, SIZE_MAX);
}

gp_size gp_text_max_width(const gp_text_style *style, unsigned int len)
{
	style = assert_style(style);

	if (len == 0)
		return 0;

	return multiply_width(style, len * style->font->max_glyph_advance) +
	       (len - 1) * style->char_xspace;
}

gp_size gp_text_avg_width(const gp_text_style *style, unsigned int len)
{
	style = assert_style(style);

	if (len == 0)
		return 0;

	return multiply_width(style, len * gp_font_avg_advance_x(style->font)) +
	       (len - 1) * style->char_xspace;
}

/*
 * Here too.
 */
gp_size gp_text_max_width_chars(const gp_text_style *style, const char *chars,
                              unsigned int len)
{
	style = assert_style(style);

	if (len == 0)
		return 0;

	if (!chars)
		return gp_text_max_width(style, len);

	return len * max_glyph_advance_x(style, chars) +
	       (len - 1) * style->char_xspace;
}

/* Ascend, Descend, Height -- far easier */

static unsigned int multiply_height(const gp_text_style *style, unsigned int h)
{
	return h * style->pixel_ymul + (h - 1) * style->pixel_yspace;
}

gp_size gp_text_height(const gp_text_style *style)
{
	style = assert_style(style);

	unsigned int height = style->font->ascend + style->font->descend;

	return multiply_height(style, height);
}

gp_size gp_text_ascent(const gp_text_style *style)
{
	style = assert_style(style);

	return multiply_height(style, style->font->ascend);
}

gp_size gp_text_descent(const gp_text_style *style)
{
	style = assert_style(style);

	return multiply_height(style, style->font->descend);
}
