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
 * Copyright (C) 2009-2010 Cyril Hrubis <metan@ucw.cz>                       *
 *                                                                           *
 *****************************************************************************/

#ifndef GP_FONT_H
#define GP_FONT_H

#include <stdint.h>

/* The smallest charset, covering only the 7-bit ASCII (0x20 .. 0x7f). */
#define GP_CHARSET_7BIT		1

/* Maximum length of the font name, author, etc. (Note: these values are
 * used by the on-disc font format.)
 */
#define GP_FONT_FAMILY_MAX	63
#define GP_FONT_NAME_MAX	63
#define GP_FONT_AUTHOR_MAX	63
#define GP_FONT_LICENSE_MAX	15

/* The current version of the on-disc font format. */
#define GP_FONT_FORMAT_VMAJOR	1
#define GP_FONT_FORMAT_VMINOR	0

/* Magic string starting the on-disc font file. */
#define GP_FONT_MAGIC		"# gfxprim font file"

/*
 * Contains font metadata.
 */
typedef struct GP_Font {

	/* Name of the font family. */
	char family[GP_FONT_NAME_MAX + 1];

	/* Font name. */
	char name[GP_FONT_NAME_MAX + 1];

	/* Name of the font author. */
	char author[GP_FONT_AUTHOR_MAX + 1];

	/* Font license (default is "GPL2"). */
	char license[GP_FONT_LICENSE_MAX + 1];

	/* Font version (incremented by font author when modifying the font data,
	 * do not confuse with format version).
	 */
	unsigned int version;

	/* The charset specifies which characters are defined by the font. */
	uint8_t charset;


	/* Height of every character in pixels. */
	uint8_t height;

	/* Height of the baseline (number of pixels from the bottom). */
	uint8_t baseline;

	/*
	 * Number of bytes for each pixel line in the character data
	 * (typically 1/8 of char_width, rounded upwards).
	 */
	uint8_t bytes_per_line;

	/* Maximum width of the character bounding box (including empty areas
	 * that are not drawn but cause other characters to shift).
	 */
	uint8_t max_bounding_width;

	/*
	 * Array of GP_CharData structures, packed together sequentially
	 * without padding.
	 *
	 * Characters are stored in encoding order. The first encoded character
	 * is 0x20 (space). A font must, at a minimum, encode all characters
	 * of the 7-bit ASCII set (0x20 .. 0x7F, inclusive).
	 */
	uint8_t *data;
} GP_Font;

/* Data describing a single character. */
typedef struct GP_CharData {

	/* Width of the character in pixels. This is the area that is drawn
	 * onto, but the real area occupied by the character can be different
	 * and is defined by pre_offset and post_offset.
	 */
	uint8_t char_width;

	/* X offset to be applied to the current position *before*
	 * drawing the character.
	 */
	int8_t pre_offset;

	/* X offset to be applied to the current position *after*
	 * the character is drawn.
	 */
	int8_t post_offset;

	/* Character bitmap (size depends on width and height). */
	uint8_t bitmap[];

} GP_CharData;

/* The default font, which is hardcoded and always available. */
extern struct GP_Font GP_default_console_font;
extern struct GP_Font GP_default_proportional_font;

/* Returns the number of bytes occupied by the GP_CharData structure
 * for this font. (Currently, all characters occupy the same space
 * regardless of proportionality.)
 */
unsigned int GP_GetCharDataSize(const GP_Font *font);

/* Returns a pointer to the character data (which start by the header)
 * of the specified character in the font data area.
 */
const GP_CharData *GP_GetCharData(const GP_Font *font, int c);

/* Returns the overall size (in bytes) occupied by all characters
 * of the font (the font metadata do not count into this value;
 * add sizeof(GP_Font) to get the complete size of the font in memory.)
 */
unsigned int GP_GetFontDataSize(const GP_Font *font);

#include "GP_RetCode.h"

GP_RetCode GP_FontLoad(GP_Font **font, const char *filename);
GP_RetCode GP_FontSave(const GP_Font *font, const char *filename);

#endif /* GP_FONT_H */
