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
 * File to be #included from line.c.
 *
 * This file is a macro template. With each inclusion, you get a definition
 * of a horizontal line drawing function in form:
 *
 * void FN_NAME(GP_TARGET_TYPE *target, long color, int x0, int x1, int y)
 *
 * These arguments must be #defined in the including file:
 *
 * 	FN_NAME
 * 		Name of the function.
 * 
 * 	BYTES_PER_PIXEL
 * 		Number of bytes per pixel of the target.
 * 		
 * 	WRITE_PIXEL
 * 		A pixel writing routine to use. Must have form
 * 		void WRITE_PIXEL(uint8_t *p, long color).
 */

#include <wchar.h>

void FN_NAME(GP_TARGET_TYPE *target, long color, int x0, int x1, int y)
{
	if (target == NULL || target->format == NULL)
		return;

	/* Ensure that x0 <= x1, swap coordinates if needed. */
	if (x0 > x1) {
		FN_NAME(target, color, x1, x0, y);
		return;
	}

	/* Get the clipping rectangle. */
	int xmin, xmax, ymin, ymax;
	GP_GET_CLIP_RECT(target, xmin, xmax, ymin, ymax);

	/* Check whether the line is not completely clipped out. */
	if (y < ymin || y > ymax || x0 > xmax || x1 < xmin)
		return;

	/* Clip the start and end of the line. */
	if (x0 < xmin) {
		x0 = xmin;
	}
	if (x1 > xmax) {
		x1 = xmax;
	}

#if BYTES_PER_PIXEL == 4

	/*
	 * Inspired by GNU libc's wmemset() (by Ulrich Drepper, licensed under LGPL).
	 * 
	 * Write the pixels in groups of four, allowing the compiler to use
	 * MMX/SSE/similar instructions if available. The last few pixels are
	 * copied normally one-by-one. (Speed gain is about 15% over a naive loop
	 * on AMD Phenom CPU.)
	 */

	uint32_t *p = (uint32_t *) GP_PIXEL_ADDR(target, x0, y);
	size_t i = 1 + x1 - x0;
	uint32_t pixel = (uint32_t) color;
	while (i >= 4) {
		p[0] = pixel;
		p[1] = pixel;
		p[2] = pixel;
		p[3] = pixel;
		p += 4;
		i -= 4;
	}
	if (i > 0) {
		p[0] = pixel;
		if (i > 1) {
			p[1] = pixel;
			if (i > 2) {
				p[2] = pixel;
			}
		}
	}

#elif BYTES_PER_PIXEL == 3

	uint8_t *bytep = GP_PIXEL_ADDR(target, x0, y);

	/* How much bytes we are offset against the 32-bit boundary. */
	int shift = ((intptr_t) bytep) % 4;

	/* How much pixels must be drawn in total. (The first pixel */
	int pixelcount = 1 + x1 - x0;

	/*
	 * Pixels remaining to draw (one less than pixelcount because
	 * one incomplete pixel is drawn during the preparation phase.)
	 */
	int i = pixelcount - 1;

	/*
	 * Handle each color component separately.
	 * (Probably they are R, G, B but who knows.)
	 */
	uint8_t a = color & 0xff;
	uint8_t b = (color >> 8) & 0xff;
	uint8_t c = (color >> 16) & 0xff;

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

#elif BYTES_PER_PIXEL == 2

	uint16_t *p = (uint16_t *) GP_PIXEL_ADDR(target, x0, y);
	size_t i = 1 + x1 - x0;
	uint16_t pixel = (uint16_t) color;
	while (i >= 4) {
		p[0] = pixel;
		p[1] = pixel;
		p[2] = pixel;
		p[3] = pixel;
		p += 4;
		i -= 4;
	}
	if (i > 0) {
		p[0] = pixel;
		if (i > 1) {
			p[1] = pixel;
			if (i > 2) {
				p[2] = pixel;
			}
		}
	}

#else

	uint8_t *p = GP_PIXEL_ADDR(target, x0, y);
	uint8_t *p_end = p + (x1 - x0) * BYTES_PER_PIXEL;
	for (; p <= p_end; p += BYTES_PER_PIXEL)
		WRITE_PIXEL(p, color);

#endif
}

