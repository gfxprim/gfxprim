/*
 * This file is part of the gfxprim library.
 *
 * Copyright (c) 2009 Jiri "BlueBear" Dluhos <jiri.bluebear.dluhos@gmail.com>,
 *                    Cyril Hrubis (Metan) <metan@ucw.cz>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDERS
 * OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef GP_FONT_H
#define GP_FONT_H

#include <stdint.h>
#include <SDL/SDL.h>

/* The smallest charset, covering only the 7-bit ASCII (0x20 .. 0x7f). */
#define GP_CHARSET_7BIT		1

typedef struct {

	/* The charset specifies which characters are defined by the font. */
	uint8_t charset;

	/*
	 * Array of character bitmaps.
	 *
	 * Characters are stored sequentially. The first encoded character
	 * is 0x20 (space). A font must, at a minimum, encode all characters
	 * of the 7-bit ASCII set (0x20 .. 0x7F, inclusive).
	 */
	uint8_t * data;

	/* Maximum width of a character, in pixels. */
	uint8_t char_width;

	/*
	 * Additional horizontal space drawn between characters
	 * (in pixels, can be 0).
	 */
	uint8_t hspace;

	/* Height of every character in pixels. */
	uint8_t height;

	/*
	 * Number of bytes for each pixel line in the character data
	 * (typically 1/8 of char_width, rounded upwards).
	 */
	uint8_t bytes_per_line;
}
GP_Font;

/* The default font, which is hardcoded and always available. */
extern GP_Font GP_default_console_font;
extern GP_Font GP_default_proportional_font;

typedef struct {

	/* Font to use, or NULL to use the default font. */
	GP_Font * font;

	/* Color for the text pixels. */
	long foreground;

	/* Color for the background. */
	long background;

	/* Width, in pixels, per each pixel in the character (default is 1). */
	int pixel_width;

	/* Vertical spacing between pixels (0 if none). */
	int pixel_vspace;

	/* Horizontal spacing between pixels (0 if none). */
	int pixel_hspace;
}
GP_TextStyle;

/*
 * Static initializer for initializing a GP_TextStyle structure to default
 * values.
 * Note that at least the colors should always be changed afterwards,
 * as there is no sensible default (they are initialized to 0).
 */
#define GP_DEFAULT_TEXT_STYLE { &GP_default_console_font, 0, 0, 1, 0, 0 }

void GP_Text(SDL_Surface * surf, const GP_TextStyle * style,
		int x, int y, const char * text);

int GP_CalcTextWidth(const GP_TextStyle * style, const char * text);

#endif

