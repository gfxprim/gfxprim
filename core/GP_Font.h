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
#define FONT_FAMILY_MAX		63
#define FONT_NAME_MAX		63
#define FONT_AUTHOR_MAX		63
#define FONT_LICENSE_MAX	15

/*
 * Describes a font.
 */
typedef struct GP_Font {

	/* Name of the font family. */
	const char family[FONT_NAME_MAX + 1];

	/* Font name. */
	const char name[FONT_NAME_MAX + 1];

	/* Name of the font author. */
	const char author[FONT_AUTHOR_MAX + 1];

	/* Font license (default is "GPL2"). */
	const char license[FONT_LICENSE_MAX + 1];

	/* The charset specifies which characters are defined by the font. */
	uint8_t charset;

	/*
	 * Array of character bitmaps.
	 *
	 * Characters are stored sequentially. The first encoded character
	 * is 0x20 (space). A font must, at a minimum, encode all characters
	 * of the 7-bit ASCII set (0x20 .. 0x7F, inclusive).
	 */
	uint8_t *data;

	/* Maximum width of a character, in pixels. */
	uint8_t char_width;

	/*
	 * Additional horizontal space drawn between characters
	 * (in pixels, can be 0).
	 */
	uint8_t hspace;

	/* Height of every character in pixels. */
	uint8_t height;

	/* Height of the baseline (number of pixels from the bottom). */
	uint8_t baseline;

	/*
	 * Number of bytes for each pixel line in the character data
	 * (typically 1/8 of char_width, rounded upwards).
	 */
	uint8_t bytes_per_line;
} GP_Font;

/* The default font, which is hardcoded and always available. */
extern struct GP_Font GP_default_console_font;
extern struct GP_Font GP_default_proportional_font;

#include "GP_RetCode.h"

GP_RetCode GP_FontLoad(GP_Font *font, const char *filename);
GP_RetCode GP_FontSave(const GP_Font *font, const char *filename);

#endif /* GP_FONT_H */
