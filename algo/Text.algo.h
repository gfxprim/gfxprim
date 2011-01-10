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
 * A bitmap text drawing algorithm.
 */

#define DEF_TEXT_FN(FN_NAME, CONTEXT_T, PIXVAL_T, HLINE) \
void FN_NAME(CONTEXT_T context, const GP_TextStyle *style, int x, int y, \
	const char *str, PIXVAL_T pixval) \
{ \
	int bytes_per_char = GP_GetCharByteSize(style->font); \
\
	/* Remember the original starting height. */ \
	int y0 = y; \
\
	const char *p; \
	for (p = str; *p != '\0'; p++) { \
		int char_index = ((int) *p) - 0x20; \
\
		/* Calculate the address of the character data. */ \
		const uint8_t *src = style->font->data \
				+ char_index * bytes_per_char; \
\
		/* Character metadata: first byte is the width in pixels, \
		 * second is the left margin. \
		 */ \
		const uint8_t char_width = *src; src++; \
		const uint8_t lmargin = *src; src++; \
		const int8_t pre_offset = *src; src++; \
		const int8_t post_offset = *src; src++; \
\
		/* Starting and final X for each character line. */ \
		int x0 = x + pre_offset * (style->pixel_xmul + style->pixel_xspace); \
		int x1 = x0 + char_width * (style->pixel_xmul + style->pixel_xspace); \
\
		/* Draw the character line by line. */ \
		int line, linerep, i; \
		for (line = 0, y = y0; line < style->font->height; line++, y += style->pixel_yspace) { \
\
			/* repeat the line as specified by pixel_ymul */ \
			for (linerep = 0; linerep < style->pixel_ymul; linerep++, y++) { \
				uint8_t const * linesrc = src + line * style->font->bytes_per_line; \
				uint8_t mask = 0x80; \
\
				/* skip left margin */ \
				for (i = 0; i < lmargin; i++) { \
					mask >>= 1; \
					if (mask == 0) { \
						linesrc++; \
						mask = 0x80; \
					} \
				} \
\
				/* draw the line of the character bitmap */ \
				for (x = x0; x < x1; x += style->pixel_xmul + style->pixel_xspace) { \
					if (*linesrc & mask) { \
						HLINE(context, \
							x, x + style->pixel_xmul - 1, \
							y, pixval); \
					} \
					mask >>= 1; \
					if (mask == 0) { \
						linesrc++; \
						mask = 0x80; \
					} \
				} \
			} \
		} \
\
		/* Update the X position. */ \
		x = x0 + post_offset * (style->pixel_xmul + style->pixel_xspace);  \
		x += style->font->hspace * style->pixel_xmul;	/* optional extra spacing */ \
	} \
}

