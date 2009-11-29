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

#include "GP.h"

/*
 * Draws a single character, using (x0, y0) as the topleft corner
 * of the character cell.
 */
static void GP_PutChar(SDL_Surface * surf, const GP_TextStyle * style, int x0, int y0,
			const uint8_t * char_data)
{
	int pixel_width = style->pixel_width;
	int xdelta = style->pixel_width + style->pixel_hspace;
	int ydelta = 1 + style->pixel_vspace;

	int x1 = x0 + style->font->char_width * xdelta;
	int y1 = y0 + style->font->height * ydelta;

	long foreground = style->foreground;

	int x, y;
	const uint8_t * src = char_data;
	uint8_t mask = 0x80;
	for (y = y0; y < y1; y += ydelta) {
		for (x = x0; x < x1; x += xdelta) {
			if (*src & mask) {
				GP_HLine(surf, foreground, x,
					x + pixel_width - 1, y);
			}
			mask >>= 1;
			if (mask == 0) {
				src++;
				mask = 0x80;
			}
		}

		/* The next row always starts at the byte boundary. */
		if (mask != 0x80) {
			src++;
			mask = 0x80;
		}
	}
}

void GP_Text(SDL_Surface * surf, const GP_TextStyle * style,
		int x, int y, const char * str)
{
	if (surf == NULL || style == NULL || style->font == NULL || str == NULL)
		return;

	int bytes_per_char = style->font->bytes_per_line * style->font->height;

	int hstep = (style->font->char_width + style->font->hspace) * style->pixel_width
			+ style->font->char_width * style->pixel_hspace;

	const char * p = str;
	while (*p != '\0') {
		int char_index = ((int) *p) - 0x20;

		const uint8_t * char_data = style->font->data
				+ char_index * bytes_per_char;

		GP_PutChar(surf, style, x, y, char_data);

		/* go to the next character */
		x += hstep;
		p++;
	}
}

