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
static int GP_PutChar(SDL_Surface *surf, const GP_TextStyle *style,
		int x0, int y0, const uint8_t *char_data)
{
	int pixel_width = style->pixel_width;
	int xdelta = style->pixel_width + style->pixel_hspace;
	int ydelta = 1 + style->pixel_vspace;

	const uint8_t *src = char_data;

	/* The first byte specifies width in pixels. */
	const uint8_t char_width = *src;
	src++;

	/* Next byte specifies the left margin. */
	const uint8_t lmargin = *src;
	src++;

	int x1 = x0 + char_width * xdelta;
	int y1 = y0 + style->font->height * ydelta;

	long foreground = style->foreground;

/* Advances the mask by a single bit. */
#define ADVANCE_MASK { \
	mask >>= 1; \
	if (mask == 0) { \
		src++; \
		mask = 0x80; \
	} \
}

/* Ensures that the mask is at the start of the next byte. */
#define ALIGN_MASK_TO_BYTE_BOUNDARY { \
	if (mask != 0x80) { \
		src++; \
		mask = 0x80; \
	} \
}

	int i, x, y;
	uint8_t mask = 0x80;
	for (y = y0; y < y1; y += ydelta) {

		/* Skip the left margin pixels. */
		for (i = 0; i < lmargin; i++) {
			ADVANCE_MASK;
		}

		/* Draw the line of pixels. */
		for (x = x0; x < x1; x += xdelta) {
			if (*src & mask) {
				GP_HLine(surf, foreground, x,
					x + pixel_width - 1, y);
			}
			ADVANCE_MASK;
		}

		/* Each pixel line starts at a byte boundary. */
		ALIGN_MASK_TO_BYTE_BOUNDARY;
	}

	return x + style->font->hspace * style->pixel_width;
}

void GP_Text(SDL_Surface *surf, const GP_TextStyle *style,
		int x, int y, const char *str)
{
	if (surf == NULL || style == NULL || style->font == NULL || str == NULL)
		return;

	int bytes_per_char = 2 + style->font->bytes_per_line * style->font->height;

	const char * p;
	for (p = str; *p != '\0'; p++) {
		int char_index = ((int) *p) - 0x20;

		const uint8_t * char_data = style->font->data
				+ char_index * bytes_per_char;

		x = GP_PutChar(surf, style, x, y, char_data);
	}
}

static int GP_CharWidth(const GP_TextStyle *style, char c)
{
	int bytes_per_char = 2 + style->font->bytes_per_line * style->font->height;

	const uint8_t * char_data = style->font->data + ((int) c - 0x20) * bytes_per_char;

	/* The first byte specifies width in pixels. */
	const uint8_t char_width = *char_data;

	return char_width * (style->pixel_width + style->pixel_hspace);
}

int GP_TextWidth(const GP_TextStyle *style, const char *str)
{
	if (style == NULL || str == NULL)
		return 0;

	int width = 0;

	const char *p;
	for (p = str; *p; p++) {
		width += style->font->hspace * style->pixel_width;
		width += GP_CharWidth(style, *p);
	}

	return width;
}

