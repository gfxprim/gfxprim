// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2009-2014 Cyril Hrubis <metan@ucw.cz>
 */

#include <stdlib.h>
#include <text/gp_font.h>
#include <text/gp_fonts.h>

uint32_t gp_get_glyph_count(gp_char_set charset)
{
	switch (charset) {
	case GP_CHARSET_7BIT:
		return 0x7f - 0x20 + 1;
	break;
	default:
		return 0;
	}
}

gp_glyph *gp_get_glyph(const gp_font_face *font, int c)
{
	int i;

	switch (font->charset) {
	case GP_CHARSET_7BIT:
		if (c < 0x20 || c > 0x7f)
			return NULL;
		i = c - ' ';
	break;
	default:
		return NULL;
	}

	uint32_t offset;

	if (font->glyph_offsets[0] == 0)
		offset = font->glyph_offsets[i];
	else
		offset = font->glyph_offsets[0] * i;

	return (gp_glyph*)(font->glyphs + offset);
}

void gp_font_face_free(gp_font_face *self)
{
	if (!self)
		return;

	free(self->glyphs);
	free(self);
}
