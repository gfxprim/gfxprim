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

/*
 * Parameterized template for text drawing function.
 * Parameters that must be #defined outside:
 *
 * 	FN_ATTR
 * 		(Optional.) Attributes of the function (e.g. "static").
 * 	FN_NAME
 * 		Name of the function.
 */

#ifndef FN_ATTR
#define FN_ATTR
#endif

static int GP_PutChar(GP_TARGET_TYPE *surf, GP_COLOR_TYPE color,
	const struct GP_TextStyle *style, int x0, int y0,
	const uint8_t *char_data)
{
	int xdelta = style->pixel_xmul + style->pixel_xspace;

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
					GP_BACKEND_HLINE(surf, color,
						x, x + style->pixel_xmul - 1,
						y);
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

FN_ATTR void FN_NAME(GP_TARGET_TYPE *target, GP_COLOR_TYPE color,
	const struct GP_TextStyle *style, int x, int y, const char *str)
{
	int bytes_per_char = 2 + style->font->bytes_per_line * style->font->height;

	const char *p;
	for (p = str; *p != '\0'; p++) {
		int char_index = ((int) *p) - 0x20;

		const uint8_t *char_data = style->font->data
				+ char_index * bytes_per_char;

		x = GP_PutChar(target, color, style, x, y, char_data);
	}
}


#undef FN_ATTR
#undef FN_NAME
