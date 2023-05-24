// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2009-2012 Cyril Hrubis <metan@ucw.cz>
 */

#ifndef TEXT_GP_FONT_H
#define TEXT_GP_FONT_H

#include <stddef.h>
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

/*
 * Font style bitflags.
 */
typedef enum gp_font_style {
	GP_FONT_REGULAR = 0x00,
	GP_FONT_MONO = 0x01,
	GP_FONT_BOLD = 0x02,
	GP_FONT_ITALIC = 0x04,
	GP_FONT_STYLE_MASK = 0x0f,
	/*
	 * If passed make sure we return something
	 * possibly the default compiled-in font
	 */
	GP_FONT_FALLBACK = 0x10,
} gp_font_style;

#define GP_FONT_STYLE(x) ((x) & GP_FONT_STYLE_MASK)

typedef uint32_t gp_glyph_offset;
#define GP_NOGLYPH UINT32_MAX

typedef struct gp_glyphs {
	/* Pointer to glyph bitmap buffer */
	void *glyphs;

	/*
	 * Offsets to the glyph data.
	 *
	 * If offsets is NULL offset is the same for all glyphs and saved in
	 * offset instead.
	 */
	gp_glyph_offset *offsets;
	gp_glyph_offset offset;

	/*
	 * First and last character in glyphs table
	 *
	 * For table 0 which is ASCII this is set to 0x20 and 0x7f
	 */
	uint32_t min_glyph;
	uint32_t max_glyph;
} gp_glyphs;

typedef struct gp_font_face gp_font_face;

/**
 * font may introduce a "lazy" loader that will load glyphs 'on demand'.
 */
typedef struct gp_font_face_ops {
	gp_glyph *(*glyph_load)(const gp_font_face *self, uint32_t ch);
	void (*font_free)(gp_font_face *self);
} gp_font_face_ops;

struct gp_font_face {
	/* Font family name - eg. Sans, Serif ... */
	char family_name[GP_FONT_NAME_MAX];

	/* Font style flags */
	uint8_t style;

	/* Size of the glyphs array */
	uint8_t glyph_tables;

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
	 * Average glyph advance.
	 */
	uint16_t avg_glyph_advance;

	/*
	 * Bitmap format for all glyphs
	 */
	gp_font_bitmap_format glyph_bitmap_format;

	/*
	 * "Lazy" loader used for non-ascii unicode glyphs
	 */
	const gp_font_face_ops *ops;
	void *priv;

	/* Glyphs tables
	 *
	 * NULL terminated array of glyph tables sorted by the max_glyph, i.e.
	 * the ASCII which ends at 0x7f should be first.
	 */
	gp_glyphs glyphs[];
};

/*
 * All fonts must include LATIN_BASIC and it has to be the first table
 * in the font.
 */
typedef enum gp_font_ucode_block {
	/* 0x20 - 0x7f */
	GP_UCODE_LATIN_BASIC = 0x0001,
	/* 0xa0 - 0xff */
	GP_UCODE_LATIN_SUP = 0x0002,
	/* 0x100 - 0x17e */
	GP_UCODE_LATIN_EXT_A = 0x0004,
	/* 0x384 - 0x3ce */
	GP_UCODE_GREEK = 0x0008,
	/* 0x340 - 0x45f */
	GP_UCODE_CYRILIC = 0x0010,
	/* 0x3041 - 0x3096 */
	GP_UCODE_HIRAGANA= 0x0020,
	/* 0x30a0 - 0x30aff */
	GP_UCODE_KATAKANA = 0x0030,
} gp_font_ucode_block;

/*
 * Font family is a group of fonts of the same family and size but different
 * style i.e. monospace, bold, italic...
 *
 * The fonts array is NULL terminated.
 */
typedef struct gp_font_family {
	const char *family_name;
	/* Bitmask of unicode blocks included in the font */
	uint32_t ucode_blocks;
	const gp_font_face *const fonts[];
} gp_font_family;

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

static inline unsigned int gp_font_avg_advance_x(const gp_font_face *font)
{
	if (font->avg_glyph_advance)
		return font->avg_glyph_advance;

	/* For monospace bitmap fonts the avg == max */
	return font->max_glyph_advance;
}

static inline const char *gp_font_family_name(const gp_font_face *font)
{
	return font->family_name;
}

const char *gp_font_style_name(uint8_t style);

/*
 * Returns glyph mapping
 */
gp_glyph *gp_get_glyph(const gp_font_face *font, uint32_t ch);

/* Loads font face from file  */
gp_font_face *gp_font_face_load(const char *path, uint32_t width, uint32_t height);

/* Uses fontconfig to lookup font file */
gp_font_face *gp_font_face_fc_load(const char *name, uint32_t width, uint32_t height);

/* Free the font face memory */
void gp_font_face_free(gp_font_face *self);

#endif /* TEXT_GP_FONT_H */
