// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2009-2021 Cyril Hrubis <metan@ucw.cz>
 */

#include <stdlib.h>
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

gp_glyph *gp_get_glyph(const gp_font_face *font, int c)
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

const char *gp_font_style_name(uint8_t style)
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

void gp_font_face_free(gp_font_face *self)
{
	size_t i;

	if (!self)
		return;

	for (i = 0; i < self->glyph_tables; i++) {
		free(self->glyphs[i].offsets);
		free(self->glyphs[i].glyphs);
	}

	free(self);
}
