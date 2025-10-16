// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2009-2021 Cyril Hrubis <metan@ucw.cz>
 */

#include <stdlib.h>
#include <utils/gp_utf.h>
#include <text/gp_font.h>
#include <text/gp_fonts.h>

static gp_glyph *get_glyph_from_table(const gp_glyphs *glyphs, uint32_t pos)
{
	uint32_t offset;

	if (!glyphs->offsets) {
		offset = glyphs->offset * pos;
	} else {
		offset = glyphs->offsets[pos];

		if (offset == GP_NOGLYPH)
			return NULL;
	}

	return (gp_glyph*)(glyphs->glyphs + offset);
}

static gp_glyph *glyph_get(const gp_font_face *font, uint32_t c)
{
	uint8_t i;

	if (c >= 0x20 && c <= 0x7f)
		return get_glyph_from_table(font->glyphs, c - 0x20);

	for (i = 1; i < font->glyph_tables; i++) {
		uint32_t min_glyph = font->glyphs[i].min_glyph;
		uint32_t max_glyph = font->glyphs[i].max_glyph;

		if (c >= min_glyph && c <= max_glyph)
			return get_glyph_from_table(font->glyphs + i, c - min_glyph);
	}

	return NULL;
}

gp_glyph *gp_glyph_get(const gp_font_face *font, uint32_t ch)
{
	gp_glyph *glyph = glyph_get(font, ch);

	if (!glyph && font->ops && font->ops->glyph_load)
		glyph = font->ops->glyph_load(font, ch);

	if (!glyph && ch > 0x7f) {
		uint32_t fb = gp_utf_fallback(ch);

		if (fb != ch)
			glyph = glyph_get(font, fb);
	}

	if (!glyph)
		glyph = glyph_get(font, '?');

	return glyph;
}

const char *gp_font_style_name(gp_font_style style)
{
	switch (GP_FONT_STYLE(style)) {
	case GP_FONT_REGULAR:
		return "Regular";
	case GP_FONT_REGULAR|GP_FONT_BOLD:
		return "Regular:Bold";
	case GP_FONT_REGULAR|GP_FONT_ITALIC:
		return "Regular:Italic";
	case GP_FONT_REGULAR|GP_FONT_BOLD|GP_FONT_ITALIC:
		return "Regular:Bold:Italic";
	case GP_FONT_MONO:
		return "Monospace";
	case GP_FONT_MONO|GP_FONT_BOLD:
		return "Monospace:Bold";
	case GP_FONT_MONO|GP_FONT_ITALIC:
		return "Monospace:Italic";
	case GP_FONT_MONO|GP_FONT_BOLD|GP_FONT_ITALIC:
		return "Monospace:Bold:Italic";
	default:
		return "???";
	}
}

const char *gp_font_ucode_block_name(enum gp_font_ucode_block block)
{
	switch (block) {
	case GP_UCODE_LATIN_BASIC:
		return "Basic Latin";
	case GP_UCODE_LATIN_SUP:
		return "Latin Supplementary";
	case GP_UCODE_LATIN_EXT_A:
		return "Latin Extended A";
	case GP_UCODE_GREEK:
		return "Greek";
	case GP_UCODE_CYRILIC:
		return "Cyrilic";
	case GP_UCODE_PUNCTUATION:
		return "Punctuation";
	case GP_UCODE_HIRAGANA:
		return "Hiragana";
	case GP_UCODE_KATAKANA:
		return "Katakana";
	}

	return "???";
}

void gp_font_face_free(gp_font_face *self)
{
	if (!self)
		return;

	if (!self->ops->font_free)
		return;

	self->ops->font_free(self);
}
