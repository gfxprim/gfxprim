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

#ifndef TEXT_GP_FONT_H
#define TEXT_GP_FONT_H

#include <stdint.h>

#define GP_FONT_NAME_MAX 64

/*
 * Data describing single Glyph.
 *
 * Note that glyph do not necessarily correspond to one character (for example
 * ligature is a glyph but corresponds to at least two characters).
 *
 * The glyphs are rendered to horizontal baseline, vertical rendering is not
 * supported.
 *
 * The structure could contain glyphs of different BPP and information about
 * the bitmap format is stored in the font structure. The bitmap lines are byte
 * aligned.
 */
typedef struct GP_GlyphBitmap {
	/*
	 * Bitmap width in pixels.
	 */
	uint8_t width;

	/*
	 * Bitmap heigth in pixels.
	 */
	uint8_t height;

	/*
	 * X offset to be applied before we start drawing.
	 */
	int8_t bearing_x;

	/*
	 * Y offset from baseline to the top of the bitmap.
	 */
	int8_t bearing_y;

	/*
	 * Offset to be applied after drawing, defines
	 * basepoint for next glyph.
	 */
	uint8_t advance_x;

	/*
	 * Character bitmap, byte aligned bitmap.
	 */
	uint8_t bitmap[];
} GP_GlyphBitmap;

typedef enum GP_CharSet {
	GP_CHARSET_7BIT,
} GP_CharSet;

/*
 * Glyph bitmap data format.
 *
 * The bitmap is byte aligned and for 1BPP the number of bytes per row is
 * rounted to bytes.
 *
 */
typedef enum GP_FontBitmapFormat {
	GP_FONT_BITMAP_1BPP,
	GP_FONT_BITMAP_8BPP,
} GP_FontBitmapFormat;

/*
 * Font face
 */
typedef struct GP_FontFace {
	/*
	 * Font family name - eg. Sans, Serif ...
	 */
	char family_name[GP_FONT_NAME_MAX];

	/*
	 * Font style name - Medium, Bold, Italic ...
	 */
	char style_name[GP_FONT_NAME_MAX];

	/*
	 * Enum for supported charsets.
	 */
	uint8_t charset;

	/*
	 * Maximal height of font glyph from baseline to the top.
	 */
	uint16_t ascend;

	/*
	 * Maximal length of font glyph from baseline to the bottom.
	 */
	uint16_t descend;

	/*
	 * Maximal width of font glyph.
	 *
	 * (basically max from glyph->width + glyph->bearing_x)
	 */
	uint16_t max_glyph_width;

	/*
	 * Maximal glyph advance.
	 */
	uint16_t max_glyph_advance;

	/*
	 * Bitmap format for all glyphs
	 */
	GP_FontBitmapFormat glyph_bitmap_format;

	/*
	 * Pointer to glyph bitmap buffer.
	 */
	void *glyphs;

	/*
	 * Offsets to the glyph data.
	 *
	 * If glyph_offset[0] == 0, the table glyph_offsets holds
	 * offsets for all characters in glyphs. Otherwise the
	 * glyph_offset[0] defines step in the glyph table.
	 */
	uint32_t glyph_offsets[];
} GP_FontFace;

/*
 * Returns font height eg. ascend + descend
 */
static inline unsigned int GP_FontHeight(const GP_FontFace *font)
{
	return font->ascend + font->descend;
}

static inline unsigned int GP_FontAscend(const GP_FontFace *font)
{
	return font->ascend;
}

static inline unsigned int GP_FontDescend(const GP_FontFace *font)
{
	return font->descend;
}

static inline unsigned int GP_FontMaxWidth(const GP_FontFace *font)
{
	return font->max_glyph_width;
}

static inline unsigned int GP_FontMaxAdvanceX(const GP_FontFace *font)
{
	return font->max_glyph_advance;
}

static inline const char *GP_FontFamily(const GP_FontFace *font)
{
	return font->family_name;
}

static inline const char *GP_FontStyle(const GP_FontFace *font)
{
	return font->style_name;
}

/*
 * Returns glyph count for charset.
 */
uint32_t GP_GetGlyphCount(GP_CharSet charset);

/*
 * Returns glyph mapping
 */
GP_GlyphBitmap *GP_GetGlyphBitmap(const GP_FontFace *font, int c);

/*
 * Loads font face from file.
 */
GP_FontFace *GP_FontFaceLoad(const char *path, uint32_t width, uint32_t height);

/*
 * Free the font face memory.
 */
void GP_FontFaceFree(GP_FontFace *self);

#endif /* TEXT_GP_FONT_H */
