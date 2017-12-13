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
typedef struct gp_glyph {
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
} gp_glyph;

typedef enum gp_char_set {
	GP_CHARSET_7BIT,
} gp_char_set;

/*
 * Glyph bitmap data format.
 *
 * The bitmap is byte aligned and for 1BPP the number of bytes per row is
 * rounted to bytes.
 *
 */
typedef enum gp_font_bitmap_format {
	GP_FONT_BITMAP_1BPP,
	GP_FONT_BITMAP_8BPP,
} gp_font_bitmap_format;

typedef struct gp_font_face {
	/* Font family name - eg. Sans, Serif ... */
	char family_name[GP_FONT_NAME_MAX];

	/* Font style name - Medium, Bold, Italic ... */
	char style_name[GP_FONT_NAME_MAX];

	/* Enum for supported charsets */
	uint8_t charset;

	/* Maximal height of font glyph from baseline to the top. */
	uint16_t ascend;

	/* Maximal length of font glyph from baseline to the bottom. */
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
	gp_font_bitmap_format glyph_bitmap_format;

	/* Pointer to glyph bitmap buffer */
	void *glyphs;

	/*
	 * Offsets to the glyph data.
	 *
	 * If glyph_offset[0] == 0, the table glyph_offsets holds offsets for
	 * all characters in glyphs, the last offset i.e. offsets[len] holds
	 * the size of glyphs array.
	 *
	 * If glyph_offset[0] != 0 the glyph_offset[0] defines step in the
	 * glyph table.
	 */
	uint32_t glyph_offsets[];
} gp_font_face;

/*
 * Returns font height eg. ascend + descend
 */
static inline unsigned int gp_font_height(const gp_font_face *font)
{
	return font->ascend + font->descend;
}

static inline unsigned int gp_font_ascend(const gp_font_face *font)
{
	return font->ascend;
}

static inline unsigned int gp_font_descend(const gp_font_face *font)
{
	return font->descend;
}

static inline unsigned int gp_font_max_width(const gp_font_face *font)
{
	return font->max_glyph_width;
}

static inline unsigned int gp_font_max_advance_x(const gp_font_face *font)
{
	return font->max_glyph_advance;
}

static inline const char *gp_font_family(const gp_font_face *font)
{
	return font->family_name;
}

static inline const char *gp_font_style(const gp_font_face *font)
{
	return font->style_name;
}

/*
 * Returns glyph count for charset.
 */
uint32_t gp_get_glyph_count(gp_char_set charset);

/*
 * Returns glyph mapping
 */
gp_glyph *gp_get_glyph(const gp_font_face *font, int c);

/* Loads font face from file  */
gp_font_face *gp_font_face_load(const char *path, uint32_t width, uint32_t height);

/* Free the font face memory */
void gp_font_face_free(gp_font_face *self);

#endif /* TEXT_GP_FONT_H */
