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
 * Copyright (C) 2009-2012 Cyril Hrubis <metan@ucw.cz>                       *
 *                                                                           *
 *****************************************************************************/

#include <string.h>

#include "GP_Core.h"
#include "GP_WritePixel.h"

static const uint8_t bytes_1BPP[] = {0x00, 0xff};

void GP_WritePixels_1BPP_LE(void *start, uint8_t off,
                            size_t cnt, unsigned int val)
{
	int len = cnt;

	/* Write start of the line */
	switch (off) {
	case 0:
	break;
	case 1:
		GP_SET_BITS1_ALIGNED(1, 1, start, val);

		if (--len == 0)
			return;
	case 2:
		GP_SET_BITS1_ALIGNED(2, 1, start, val);

		if (--len == 0)
			return;
	case 3:
		GP_SET_BITS1_ALIGNED(3, 1, start, val);

		if (--len == 0)
			return;
	case 4:
		GP_SET_BITS1_ALIGNED(4, 1, start, val);

		if (--len == 0)
			return;
	case 5:
		GP_SET_BITS1_ALIGNED(5, 1, start, val);

		if (--len == 0)
			return;
	case 6:
		GP_SET_BITS1_ALIGNED(6, 1, start, val);

		if (--len == 0)
			return;
	case 7:
		GP_SET_BITS1_ALIGNED(7, 1, start, val);

		if (--len == 0)
			return;

		start++;
	break;
	}

	/* Write as many bytes as possible */
	memset(start, bytes_1BPP[val & 0x01], len/8);

	start+=len/8;

	/* And the rest */
	switch (len%8) {
	case 7:
		GP_SET_BITS1_ALIGNED(6, 1, start, val);
	case 6:
		GP_SET_BITS1_ALIGNED(5, 1, start, val);
	case 5:
		GP_SET_BITS1_ALIGNED(4, 1, start, val);
	case 4:
		GP_SET_BITS1_ALIGNED(3, 1, start, val);
	case 3:
		GP_SET_BITS1_ALIGNED(2, 1, start, val);
	case 2:
		GP_SET_BITS1_ALIGNED(1, 1, start, val);
	case 1:
		GP_SET_BITS1_ALIGNED(0, 1, start, val);
	break;
	}
}

void GP_WritePixels_1BPP_BE(void *start, uint8_t off,
                            size_t cnt, unsigned int val)
{
	int len = cnt;

	/* Write start of the line */
	switch (off) {
	case 0:
	break;
	case 1:
		GP_SET_BITS1_ALIGNED(6, 1, start, val);

		if (--len == 0)
			return;
	case 2:
		GP_SET_BITS1_ALIGNED(5, 1, start, val);

		if (--len == 0)
			return;
	case 3:
		GP_SET_BITS1_ALIGNED(4, 1, start, val);

		if (--len == 0)
			return;
	case 4:
		GP_SET_BITS1_ALIGNED(3, 1, start, val);

		if (--len == 0)
			return;
	case 5:
		GP_SET_BITS1_ALIGNED(2, 1, start, val);

		if (--len == 0)
			return;
	case 6:
		GP_SET_BITS1_ALIGNED(1, 1, start, val);

		if (--len == 0)
			return;
	case 7:
		GP_SET_BITS1_ALIGNED(0, 1, start, val);

		if (--len == 0)
			return;

		start++;
	break;
	}

	/* Write as many bytes as possible */
	memset(start, bytes_1BPP[val & 0x01], len/8);

	start+=len/8;

	/* And the rest */
	switch (len%8) {
	case 7:
		GP_SET_BITS1_ALIGNED(1, 1, start, val);
	case 6:
		GP_SET_BITS1_ALIGNED(2, 1, start, val);
	case 5:
		GP_SET_BITS1_ALIGNED(3, 1, start, val);
	case 4:
		GP_SET_BITS1_ALIGNED(4, 1, start, val);
	case 3:
		GP_SET_BITS1_ALIGNED(5, 1, start, val);
	case 2:
		GP_SET_BITS1_ALIGNED(6, 1, start, val);
	case 1:
		GP_SET_BITS1_ALIGNED(7, 1, start, val);
	break;
	}
}

static const uint8_t bytes_2BPP[] = {0x00, 0x55, 0xaa, 0xff};

void GP_WritePixels_2BPP_LE(void *start, uint8_t off,
                            size_t cnt, unsigned int val)
{
	int len = cnt;

	/* Write start of the line */
	switch (off) {
	case 0:
	break;
	case 1:
		GP_SET_BITS1_ALIGNED(2, 2, start, val);

		if (--len == 0)
			return;
	case 2:
		GP_SET_BITS1_ALIGNED(4, 2, start, val);

		if (--len == 0)
			return;
	case 3:
		GP_SET_BITS1_ALIGNED(6, 2, start, val);

		if (--len == 0)
			return;

		start++;
	break;
	}

	/* Write as many bytes as possible */
	memset(start, bytes_2BPP[val & 0x03], len/4);

	start+=len/4;

	/* And the rest */
	switch (len%4) {
	case 3:
		GP_SET_BITS1_ALIGNED(4, 2, start, val);
	case 2:
		GP_SET_BITS1_ALIGNED(2, 2, start, val);
	case 1:
		GP_SET_BITS1_ALIGNED(0, 2, start, val);
	break;
	}
}

void GP_WritePixels_2BPP_BE(void *start, uint8_t off,
                            size_t cnt, unsigned int val)
{
	int len = cnt;

	/* Write start of the line */
	switch (off) {
	case 0:
	break;
	case 1:
		GP_SET_BITS1_ALIGNED(6, 2, start, val);

		if (--len == 0)
			return;
	case 2:
		GP_SET_BITS1_ALIGNED(4, 2, start, val);

		if (--len == 0)
			return;
	case 3:
		GP_SET_BITS1_ALIGNED(2, 2, start, val);

		if (--len == 0)
			return;

		start++;
	break;
	}

	/* Write as many bytes as possible */
	memset(start, bytes_2BPP[val & 0x03], len/4);

	start+=len/4;

	/* And the rest */
	switch (len%4) {
	case 3:
		GP_SET_BITS1_ALIGNED(0, 2, start, val);
	case 2:
		GP_SET_BITS1_ALIGNED(2, 2, start, val);
	case 1:
		GP_SET_BITS1_ALIGNED(4, 2, start, val);
	break;
	}
}

static const uint8_t bytes_4BPP[] = {
	0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77,
	0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff
};

void GP_WritePixels_4BPP_LE(void *start, uint8_t off,
                            size_t cnt, unsigned int val)
{
	int len = cnt;

	/* Write start of the line */
	switch (off) {
	case 0:
	break;
	case 1:
		GP_SET_BITS1_ALIGNED(4, 4, start, val);

		if (--len == 0)
			return;

		start++;
	break;
	}

	/* Write as many bytes as possible */
	memset(start, bytes_4BPP[val & 0x0f], len/2);

	start+=len/2;

	/* And the rest */
	switch (len%2) {
	case 1:
		GP_SET_BITS1_ALIGNED(0, 4, start, val);
	break;
	}
}

void GP_WritePixels_4BPP_BE(void *start, uint8_t off,
                            size_t cnt, unsigned int val)
{
	int len = cnt;

	/* Write start of the line */
	switch (off) {
	case 0:
	break;
	case 1:
		GP_SET_BITS1_ALIGNED(0, 4, start, val);

		if (--len == 0)
			return;

		start++;
	break;
	}

	/* Write as many bytes as possible */
	memset(start, bytes_4BPP[val & 0x0f], len/2);

	start+=len/2;

	/* And the rest */
	switch (len%2) {
	case 1:
		GP_SET_BITS1_ALIGNED(4, 4, start, val);
	break;
	}
}

void GP_WritePixels_8BPP(void *start, size_t count, unsigned int value)
{

	memset(start, value, count);
}

void GP_WritePixels_16BPP(void *start, size_t count, unsigned int value)
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

void GP_WritePixels_24BPP(void *start, size_t count, unsigned int value)
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
		i--;
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
		break;
	case 1:
		break;
	case 2:
		bytep[0] = c;
		bytep++;
		i--;
		break;
	case 3:
		bytep[0] = b;
		bytep[1] = c;
		bytep += 2;
		i--;
		break;
	}

	while (i >= 0) {
		bytep[0] = a;
		bytep[1] = b;
		bytep[2] = c;
		bytep += 3;
		i--;
	}
}

void GP_WritePixels_32BPP(void *start, size_t count, unsigned int value)
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
