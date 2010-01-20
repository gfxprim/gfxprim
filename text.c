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

#include "GP_line.h"
#include "GP_text.h"

/*
 * Draws a single character, using (x0, y0) as the topleft corner
 * of the character cell.
 * Returns the X coordinate of the next position where the next character
 * can be drawn.
 */
static int GP_PutChar(GP_TARGET_TYPE *surf, GP_COLOR_TYPE color,
	const GP_TextStyle *style,
	int x0, int y0, const uint8_t *char_data)
{
	int xdelta = style->pixel_xmul + style->pixel_xspace;
	int ydelta = style->pixel_ymul + style->pixel_yspace;

	const uint8_t *src = char_data;

	/* The first byte specifies width in pixels. */
	const uint8_t char_width = *src;
	src++;

	/* Next byte specifies the left margin. */
	const uint8_t lmargin = *src;
	src++;

	/* Final X for each character line. */
	int x1 = x0 + char_width * xdelta;

	int line, linerep, i, x, y;
	for (line = 0, y = y0; line < style->font->height; line++, y += style->pixel_yspace) {

		/* repeat the line as specified by pixel_ymul */
		for (linerep = 0; linerep < style->pixel_ymul; linerep++, y++) {
			uint8_t const * linesrc = src + line * style->font->bytes_per_line;
			uint8_t mask = 0x80;

			/* skip left margin */
			for (i = 0; i < lmargin; i++) {
				mask >>= 1;
				if (mask == 0) {
					linesrc++;
					mask = 0x80;
				}
			}

			/* draw the line of the character */
			for (x = x0; x < x1; x += xdelta) {
				if (*linesrc & mask) {
					GP_HLine(surf, color,
						x, x + style->pixel_xmul - 1, y);
				}
				mask >>= 1;
				if (mask == 0) {
					linesrc++;
					mask = 0x80;
				}
			}
		}
	}

	return x + style->font->hspace * style->pixel_ymul;
}

void GP_Text(SDL_Surface *surf, GP_COLOR_TYPE color,
	const GP_TextStyle *style, int x, int y, const char *str)
{
	if (surf == NULL || style == NULL || style->font == NULL || str == NULL)
		return;

	int bytes_per_char = 2 + style->font->bytes_per_line * style->font->height;

	const char * p;
	for (p = str; *p != '\0'; p++) {
		int char_index = ((int) *p) - 0x20;

		const uint8_t * char_data = style->font->data
				+ char_index * bytes_per_char;

		x = GP_PutChar(surf, color, style, x, y, char_data);
	}
}

static int GP_CharWidth(const GP_TextStyle *style, char c)
{
	int bytes_per_char = 2 + style->font->bytes_per_line * style->font->height;

	const uint8_t * char_data = style->font->data + ((int) c - 0x20) * bytes_per_char;

	/* The first byte specifies width in pixels. */
	const uint8_t char_width = *char_data;

	return char_width * (style->pixel_xmul + style->pixel_xspace);
}

int GP_TextWidth(const GP_TextStyle *style, const char *str)
{
	if (style == NULL || str == NULL)
		return 0;

	int width = 0;

	const char *p;
	for (p = str; *p; p++) {
		width += style->font->hspace * style->pixel_xmul;
		width += GP_CharWidth(style, *p);
	}

	return width;
}

