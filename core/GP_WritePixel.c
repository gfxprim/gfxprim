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

#include "GP.h"

void GP_WritePixels8bpp(void *start, size_t count, uint8_t value)
{
	uint8_t *p = (uint8_t *) start;
	uint8_t *end = p + count;
	for (; p <= end; p++)
		*p = value;
}

void GP_WritePixels16bpp(void *start, size_t count, uint16_t value)
{
	uint16_t *p = (uint16_t *) start;
	size_t i;

	/* Write as much pixels as possible in 4-pixel blocks. */
	for (i = count; i >= 4; p += 4, i -= 4) {
		p[0] = value;
		p[1] = value;
		p[2] = value;
		p[3] = value;
	}

	/* Write the rest. */
	if (i > 0) {
		p[0] = value;
		if (i > 1) {
			p[1] = value;
			if (i > 2) {
				p[2] = value;
			}
		}
	}
}

void GP_WritePixels24bpp(void *start, size_t count, uint32_t value)
{
	uint8_t *bytep = (uint8_t *) start;

	/* How much bytes we are offset against the 32-bit boundary. */
	int shift = ((intptr_t) bytep) % 4;

	/*
	 * Pixels remaining to draw (one less than pixelcount because
	 * one incomplete pixel is drawn during the preparation phase.)
	 */
	int i = count - 1;

	/*
	 * Handle each color component separately.
	 * (Probably they are R, G, B but who knows.)
	 */
	uint8_t a = value & 0xff;
	uint8_t b = (value >> 8) & 0xff;
	uint8_t c = (value >> 16) & 0xff;

	uint32_t *p;
	uint32_t block[3];

	/*
	 * The line consists of three repeating 32-bit segments
	 * (except for the starting and ending segment:
	 * ABCA, BCAB, CABC.
	 */
#if __BIG_ENDIAN__
	uint32_t abca = a << 24 | b << 16 | c << 8 | a;
	uint32_t bcab = b << 24 | c << 16 | a << 8 | b;
	uint32_t cabc = c << 24 | a << 16 | b << 8 | c;
#else
	uint32_t abca = a << 24 | c << 16 | b << 8 | a;
	uint32_t bcab = b << 24 | a << 16 | c << 8 | b;
	uint32_t cabc = c << 24 | b << 16 | a << 8 | c;
#endif

	/*
	 * Handle the first few bytes (1 pixel or less) and prepare
	 * the repeating sequence.
	 */
	switch (shift) {
	default: /* shut up gcc */
	case 0:
		block[0] = abca;
		block[1] = bcab;
		block[2] = cabc;
		p = (uint32_t *) bytep;
		break;
	case 3:
		bytep[0] = a;
		block[0] = bcab;
		block[1] = cabc;
		block[2] = abca;
		p = (uint32_t *)(bytep + 1);
		break;
	case 2:
		bytep[0] = a;
		bytep[1] = b;
		block[0] = cabc;
		block[1] = abca;
		block[2] = bcab;
		p = (uint32_t *)(bytep + 2);
		break;
	case 1:
		bytep[0] = a;
		bytep[1] = b;
		bytep[2] = c;
		block[0] = abca;
		block[1] = bcab;
		block[2] = cabc;
		p = (uint32_t *)(bytep + 3);
		break;
	}

	/* 
	 * Write as much of the line as possible as
	 * triplets of 32-bit numbers; hopefully the compiler can
	 * put some wide write instructions in.
	 */
	while (i >= 4) {
		p[0] = block[0];
		p[1] = block[1];
		p[2] = block[2];
		p += 3;
		i -= 4;
	}

	/* Write the rest of the last pixel of the main part */
	bytep = (uint8_t *) p;
	switch (shift) {
	case 0:
		bytep[0] = a;
		bytep[1] = b;
		bytep[2] = c;
		bytep += 3;
		break;
	case 1:
		break;
	case 2:
		bytep[0] = c;
		bytep++;
		break;
	case 3:
		bytep[0] = b;
		bytep[1] = c;
		bytep += 2;
		break;
	}

	while (i > 0) {
		bytep[0] = a;
		bytep[1] = b;
		bytep[2] = c;
		bytep += 3;
		i--;
	}
}

void GP_WritePixels32bpp(void *start, size_t count, uint32_t value)
{
	/*
	 * Inspired by GNU libc's wmemset() (by Ulrich Drepper, licensed under LGPL).
	 * 
	 * Write the pixels in groups of four, allowing the compiler to use
	 * MMX/SSE/similar instructions if available. The last few pixels are
	 * copied normally one-by-one. (Speed gain is about 15% over a naive loop
	 * on AMD Phenom CPU.)
	 */

	uint32_t *p = (uint32_t *) start;
	size_t i = count;
	while (i >= 4) {
		p[0] = value;
		p[1] = value;
		p[2] = value;
		p[3] = value;
		p += 4;
		i -= 4;
	}
	if (i > 0) {
		p[0] = value;
		if (i > 1) {
			p[1] = value;
			if (i > 2) {
				p[2] = value;
			}
		}
	}
}
