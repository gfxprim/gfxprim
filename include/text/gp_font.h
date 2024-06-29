// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2009-2024 Cyril Hrubis <metan@ucw.cz>
 */

/**
 * @file gp_font.h
 * @brief An in-memory font description.
 *
 * @image html images/fonts/glyph_metrics.png
 */
#ifndef TEXT_GP_FONT_H
#define TEXT_GP_FONT_H

#include <stddef.h>
#include <stdint.h>

#define GP_FONT_NAME_MAX 64

/**
 * @brief A data describing single Glyph.
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
	/** @brief Bitmap width in pixels. */
	uint8_t width;

	/** @brief Bitmap heigth in pixels. */
	uint8_t height;

	/** @brief An X offset to be applied before we start drawing. */
	int8_t bearing_x;

	/** @brief An Y offset from baseline to the top of the bitmap. */
	int8_t bearing_y;

	/**
	 * @brief An offset to be applied after drawing.
	 *
	 * This defines a basepoint for next glyph.
	 */
	uint8_t advance_x;

	/**
	 * @brief A character bitmap.
	 *
	 * This is width * height * depth bits rounded up to closest whole
	 * byte.
         */
	uint8_t bitmap[];
} gp_glyph;

/**
 * @brief Glyph bitmap data format.
 *
 * Defines depth for the gp_glyph::bitmap.
 */
typedef enum gp_font_bitmap_format {
	/** @brief 1 bit per pixel. */
	GP_FONT_BITMAP_1BPP,
	/** @brief 8 bit per pixel. */
	GP_FONT_BITMAP_8BPP,
} gp_font_bitmap_format;

/**
 * @brief Font style bitflags.
 *
 * The monospace and regular font flags can be bitwise combined with bold and
 * italic.
 */
typedef enum gp_font_style {
	/** @brief Regular font. */
	GP_FONT_REGULAR = 0x00,
	/** @brief Monospace font. */
	GP_FONT_MONO = 0x01,
	/** @brief Bold font. */
	GP_FONT_BOLD = 0x02,
	/** @brief Italic font. */
	GP_FONT_ITALIC = 0x04,
	/** @brief Font style mask */
	GP_FONT_STYLE_MASK = 0x0f,
	/**
	 * @brief A fallback flag.
	 *
	 * If passed the font lookup functions always return some font possibly
	 * the default compiled-in font
	 */
	GP_FONT_FALLBACK = 0x10,
} gp_font_style;

#define GP_FONT_STYLE(x) ((x) & GP_FONT_STYLE_MASK)

typedef uint32_t gp_glyph_offset;
#define GP_NOGLYPH UINT32_MAX

/**
 * @brief An unicode glyphs block.
 */
typedef struct gp_glyphs {
	/**
	 * @brief Pointer to glyph bitmap buffer.
	 *
	 * This is a buffer with all glyph pixmaps.
	 */
	void *glyphs;

	/**
	 * @brief Offsets to the glyph data.
	 *
	 * If offsets is NULL offset is the same for all glyphs and saved in
	 * gp_glyphs::offset instead.
	 */
	gp_glyph_offset *offsets;
	/**
	 * @brief An offset to the glyph data.
	 *
	 * If all offsets are the same gp_glyphs::offsets is NULL and the
	 * glyphs are stored at multiples of this offset.
	 */
	gp_glyph_offset offset;

	/**
	 * @brief First character in glyphs table.
	 *
	 * For table 0 which is ASCII this is set to 0x20.
	 */
	uint32_t min_glyph;
	/**
	 * @brief Last character in glyphs table.
	 *
	 * For table 0 which is ASCII this is set to 0x7f.
	 */
	uint32_t max_glyph;
} gp_glyphs;

typedef struct gp_font_face gp_font_face;

/**
 * @brief Font loader callback.
 *
 * When TrueType font is loaded the ASCII part is pre-rendered into the memory,
 * any other characters are loaded on-demand when needed.
 */
typedef struct gp_font_face_ops {
	/**
	 * @brief Callback to load a glyph.
	 *
	 * @param self A font face.
	 * @param ch An unicode glyph.
	 *
	 * @return A pointer to loaded glyph or NULL if it's not in a font.
	 */
	gp_glyph *(*glyph_load)(const gp_font_face *self, uint32_t ch);
	/**
	 * @brief Callback to free a font.
	 *
	 * @param self A font face.
	 */
	void (*font_free)(gp_font_face *self);
} gp_font_face_ops;

/**
 * @brief A font face.
 *
 * A group of unicode blocks for a given font.
 */
struct gp_font_face {
	/** @brief Font family name - eg. Sans, Serif ... */
	char family_name[GP_FONT_NAME_MAX];

	/** @brief A #gp_font_style flags. */
	uint8_t style;

	/**
	 * @brief A number of glyph tables in this font.
	 *
	 * The size of gp_font_face::glyphs array.
	 */
	uint8_t glyph_tables;

	/** @brief Maximal height of font glyph from baseline to the top. */
	uint16_t ascent;

	/** @brief Maximal length of font glyph from baseline to the bottom. */
	uint16_t descent;

	/**
	 * @brief Maximal width of font glyph.
	 *
	 * Basically maximum of glyph->width + glyph->bearing_x for all font
	 * glyphs.
	 */
	uint16_t max_glyph_width;

	/**
	 * @brief Maximal glyph advance.
	 */
	uint16_t max_glyph_advance;

	/**
	 * @brief An average glyph advance.
	 */
	uint16_t avg_glyph_advance;

	/**
	 * @brief Bitmap format for all glyphs.
	 */
	gp_font_bitmap_format glyph_bitmap_format;

	/**
	 * @brief On demand loader used for non-ASCII unicode glyphs.
	 */
	const gp_font_face_ops *ops;
	void *priv;

	/**
	 * @brief Glyph tables.
	 *
	 * NULL terminated array of glyph tables sorted by the max_glyph, i.e.
	 * the ASCII which ends at 0x7f should be first.
	 */
	gp_glyphs glyphs[];
};

/**
 * @brief Unicode block ids.
 *
 * All fonts must include LATIN_BASIC and it has to be the first table
 * in the font.
 */
typedef enum gp_font_ucode_block {
	/** @brief ASCII block 0x20 - 0x7f */
	GP_UCODE_LATIN_BASIC = 0x0001,
	/** @brief Latin suplement 0xa0 - 0xff */
	GP_UCODE_LATIN_SUP = 0x0002,
	/** @brief Latin extended A 0x100 - 0x17e */
	GP_UCODE_LATIN_EXT_A = 0x0004,
	/** @brief Greek 0x384 - 0x3ce */
	GP_UCODE_GREEK = 0x0008,
	/** @brief Cyrilic 0x340 - 0x45f */
	GP_UCODE_CYRILIC = 0x0010,
	/** @brief Hiragan 0x3041 - 0x3096 */
	GP_UCODE_HIRAGANA= 0x0020,
	/** @brief Katakana 0x30a0 - 0x30aff */
	GP_UCODE_KATAKANA = 0x0030,
} gp_font_ucode_block;

/**
 * @brief A font family.
 *
 * Font family is a group of font faces of the same family and size but
 * different style i.e. monospace, bold, italic...
 *
 * The fonts array is NULL terminated.
 */
typedef struct gp_font_family {
	/** @brief A font family name. */
	const char *family_name;
	/** @brief A bitmask of unicode blocks included in the font faces. */
	uint32_t ucode_blocks;
	/** Array of font faces in the font family. */
	const gp_font_face *const fonts[];
} gp_font_family;

/**
 * @brief Returns font height.
 *
 * @attention This call returns font height, the rendered text height may be
 *            different due to #gp_text_style. In most cases you should use
 *            gp_text_height() instead.
 *
 * @param font A font face.
 *
 * @return A font height, eg. ascent + descent.
 */
static inline unsigned int gp_font_height(const gp_font_face *font)
{
	return font->ascent + font->descent;
}

/**
 * @brief Returns font ascent.
 *
 * @attention This call returns font ascent, the rendered text height may be
 *            different due to #gp_text_style. In most cases you should use
 *            gp_text_ascent() instead.
 *
 * @param font A font face.
 *
 * @return A font ascent.
 */
static inline unsigned int gp_font_ascent(const gp_font_face *font)
{
	return font->ascent;
}

/**
 * @brief Returns font ascent.
 *
 * @attention This call returns font descent, the rendered text height may be
 *            different due to #gp_text_style. In most cases you should use
 *            gp_text_descent() instead.
 *
 * @param font A font face.
 *
 * @return A font ascent.
 */
static inline unsigned int gp_font_descent(const gp_font_face *font)
{
	return font->descent;
}

/**
 * @brief Returns maximal glyph width.
 *
 * For monospace fonts all letter widths are the same.
 *
 * @param font A font face.
 *
 * @return A maximal glyph width.
 */
static inline unsigned int gp_font_max_width(const gp_font_face *font)
{
	return font->max_glyph_width;
}

/**
 * @brief Returns maximal glyph advance.
 *
 * For monospace fonts all advances are the same.
 *
 * @param font A font face.
 *
 * @return A maximal glyph width.
 */
static inline unsigned int gp_font_max_advance_x(const gp_font_face *font)
{
	return font->max_glyph_advance;
}

/**
 * @brief Returns average glyph advance.
 *
 * For monospace fonts all advances are the same.
 *
 * @param font A font face.
 *
 * @return A maximal glyph width.
 */
static inline unsigned int gp_font_avg_advance_x(const gp_font_face *font)
{
	if (font->avg_glyph_advance)
		return font->avg_glyph_advance;

	/* For monospace bitmap fonts the avg == max */
	return font->max_glyph_advance;
}

/**
 * @brief Returns font family name.
 *
 * @param font A font face.
 *
 * @return A font family name.
 */
static inline const char *gp_font_family_name(const gp_font_face *font)
{
	return font->family_name;
}

/**
 * @brief Returns font style name.
 *
 * Returns font style name for a bitwise combination of the GP_FONT_STYLE_MASK
 * bits.
 *
 * @param style A font style.
 *
 * @return A font style name.
 */
const char *gp_font_style_name(gp_font_style style);

/**
 * @brief Looks up a glyph in a font.
 *
 * @param font A font face.
 * @param ch An unicode glyph.
 *
 * @return A glyph structure.
 */
gp_glyph *gp_glyph_get(const gp_font_face *font, uint32_t ch);

/**
 * @brief Load a font face from a file.
 *
 * Loads a font using FreeType library. Only one of the width and height should
 * be non-zero, to keep the correct aspect ratio for the glyph.
 *
 * @param path A path to a font file.
 * @param width A requested width.
 * @param height A requested height.
 *
 * @return A font or NULL in a case of a failure.
 */
gp_font_face *gp_font_face_load(const char *path, uint32_t width, uint32_t height);

/**
 * @brief Uses fontconfig to lookup font file.
 *
 * Looks up a font path given a font name and loads it with gp_font_face_load().
 *
 * @param name A font name such as "Arial".
 * @param width A width passed to the gp_font_face_load().
 * @param height A height passed to the gp_font_face_load().
 *
 * @return A font or NULL in a case of a failure.
 */
gp_font_face *gp_font_face_fc_load(const char *name, uint32_t width, uint32_t height);

/**
 * @brief Frees the font face memory.
 *
 * Frees the font face memory, closes FreeType hanles, etc.
 *
 * @param self A font face.
 */
void gp_font_face_free(gp_font_face *self);

#endif /* TEXT_GP_FONT_H */
