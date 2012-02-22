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
 * Copyright (C) 2009-2012 Cyril Hrubis <metan@ucw.cz>                       *
 *                                                                           *
 *****************************************************************************/

#include "core/GP_GetPutPixel.h"
#include "core/GP_MixPixels.gen.h"
#include "GP_Font.h"

#include "GP_DefaultFont.h"

uint32_t GP_GetGlyphCount(GP_CharSet charset)
{
	switch (charset) {
	case GP_CHARSET_7BIT:
		return 0x7f - 0x20 + 1;
	break;
	default:
		return 0;
	}
}

GP_GlyphBitmap *GP_GetGlyphBitmap(const GP_FontFace *font, int c)
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

	return (GP_GlyphBitmap*)(font->glyphs + offset);
}

void GP_FontFaceFree(GP_FontFace *self)
{
	free(self->glyphs);
	free(self);
}
