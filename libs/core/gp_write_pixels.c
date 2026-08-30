// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2009-2010 Jiri "BlueBear" Dluhos
 *                         <jiri.bluebear.dluhos@gmail.com>
 *
 * Copyright (C) 2009-2025 Cyril Hrubis <metan@ucw.cz>
 */

#include <string.h>
#include <core/gp_get_set_bits.h>
#include <core/gp_get_set_bytes.h>
#include <core/gp_write_pixels.gen.h>



/*
 * The bits of a byte that pixels @first to @last of it occupy.
 *
 * A pixel is bpp bits wide, and it is the bit order that says which end of the
 * byte pixel zero sits at, so a span of pixels is a span of bits either way.
 */
static uint8_t pixel_span_mask(uint8_t bpp, int up_bit,
                               uint8_t first, uint8_t last)
{
	unsigned int lo = bpp * first;
	unsigned int hi = bpp * (last + 1);

	if (up_bit) {
		unsigned int tmp = 8 - hi;

		hi = 8 - lo;
		lo = tmp;
	}

	return ((1u << hi) - 1) & ~((1u << lo) - 1);
}

/*
 * A row of a pattern is two bytes over and over, the pattern tile is four
 * pixels wide, so that is a whole number of tiles for every pixel size a
 * pattern fits into. Up to two bits per pixel a single byte is, so the row is
 * a single byte repeated there.
 *
 * So the two ends, where the run does not start or stop on a byte boundary,
 * are a single read-modify-write each rather than a walk of the pixels in
 * them: the byte the row has for that place, under the bits that end covers.
 *
 * The row itself is built by gp_pixel_pattern_row_init_*(), which is where the
 * pattern tiles and the channel layout are known.
 */
static void write_partial_byte(uint8_t *dst, uint8_t byte, uint8_t mask,
                               uint8_t bpp, int up_bit,
                               uint8_t first, uint8_t last)
{
	uint8_t m = mask & pixel_span_mask(bpp, up_bit, first, last);

	*dst = (*dst & ~m) | (byte & m);
}

static inline void write_pattern_byte(void *start, uint8_t off, size_t cnt,
                                      const gp_pixel_pattern_row *row,
                                      uint8_t bpp, int up_bit)
{
	/* Both bytes are the same up to two bits per pixel */
	uint8_t byte = row->pixels, mask = row->mask;
	uint8_t ppb = 8 / bpp;
	uint8_t *dst = start;
	size_t len = cnt;

	if (!len)
		return;

	/* The first byte, if the run starts in the middle of one */
	if (off) {
		uint8_t last = ppb - 1;

		if (len < (size_t)(ppb - off))
			last = off + len - 1;

		write_partial_byte(dst, byte, mask, bpp, up_bit, off, last);

		len -= last - off + 1;
		dst++;

		if (!len)
			return;
	}

	/*
	 * A row with nothing transparent in it is a plain fill, which is the
	 * common case and the fast one. Otherwise the pixels the pattern
	 * leaves alone have to survive, so the byte is merged in rather than
	 * stored.
	 */
	if (mask == 0xff) {
		memset(dst, byte, len / ppb);
		dst += len / ppb;
	} else {
		size_t i;

		for (i = 0; i < len / ppb; i++, dst++)
			*dst = (*dst & ~mask) | (byte & mask);
	}

	/* And the last byte, if the run does not end on a boundary */
	len %= ppb;

	if (len)
		write_partial_byte(dst, byte, mask, bpp, up_bit, 0, len - 1);
}

/*
 * Four bits per pixel, the row is two bytes and the upper one goes into the
 * byte the run starts in, the run alternates between them from there.
 */
static inline void write_pattern_2bytes(void *start, uint8_t off, size_t cnt,
                                        const gp_pixel_pattern_row *row,
                                        int up_bit)
{
	const uint8_t bytes[2] = {row->pixels >> 8, row->pixels};
	const uint8_t mask[2] = {row->mask >> 8, row->mask};
	uint8_t *dst = start;
	unsigned int i = 0;

	if (!cnt)
		return;

	/* The first byte, if the run starts with its second pixel */
	if (off) {
		write_partial_byte(dst, bytes[0], mask[0], 4, up_bit, 1, 1);
		cnt--;
		dst++;
		i = 1;
	}

	/*
	 * With nothing transparent in the row the middle is the two bytes
	 * repeated, i.e. a run of 16 bit pixels in big endian.  That writer
	 * stores whole uint16_t, so it is started on an even address, a byte
	 * before that and whatever is left after it are written here.
	 */
	if (row->mask == 0xffff) {
		size_t words;

		if (cnt >= 2 && ((uintptr_t)dst & 1)) {
			*dst++ = bytes[i];
			cnt -= 2;
			i ^= 1;
		}

		words = cnt / 4;

		if (words) {
			gp_write_pixels_16BPP_BE(dst, words, bytes[i] << 8 | bytes[i ^ 1]);
			dst += 2 * words;
			cnt -= 4 * words;
		}

		if (cnt >= 2) {
			*dst++ = bytes[i];
			cnt -= 2;
			i ^= 1;
		}
	} else {
		for (; cnt >= 2; cnt -= 2, dst++, i ^= 1)
			*dst = (*dst & ~mask[i]) | (bytes[i] & mask[i]);
	}

	/* And the last byte, if the run ends with its first pixel */
	if (cnt)
		write_partial_byte(dst, bytes[i], mask[i], 4, up_bit, 0, 0);
}

/*
 * The pixel size and the bit order are known in each of these, so the shifts,
 * the masks of the two ends and the pixels to a byte all fold into constants,
 * and what is left is a memset between two read-modify-writes.
 */
void gp_write_pixels_pattern_1BPP_DB(void *start, uint8_t off, size_t cnt,
                                     const gp_pixel_pattern_row *row)
{
	write_pattern_byte(start, off, cnt, row, 1, 0);
}

void gp_write_pixels_pattern_1BPP_UB(void *start, uint8_t off, size_t cnt,
                                     const gp_pixel_pattern_row *row)
{
	write_pattern_byte(start, off, cnt, row, 1, 1);
}

void gp_write_pixels_pattern_2BPP_DB(void *start, uint8_t off, size_t cnt,
                                     const gp_pixel_pattern_row *row)
{
	write_pattern_byte(start, off, cnt, row, 2, 0);
}

void gp_write_pixels_pattern_2BPP_UB(void *start, uint8_t off, size_t cnt,
                                     const gp_pixel_pattern_row *row)
{
	write_pattern_byte(start, off, cnt, row, 2, 1);
}

void gp_write_pixels_pattern_4BPP_DB(void *start, uint8_t off, size_t cnt,
                                     const gp_pixel_pattern_row *row)
{
	write_pattern_2bytes(start, off, cnt, row, 0);
}

void gp_write_pixels_pattern_4BPP_UB(void *start, uint8_t off, size_t cnt,
                                     const gp_pixel_pattern_row *row)
{
	write_pattern_2bytes(start, off, cnt, row, 1);
}

static const uint8_t bytes_1BPP[] = {0x00, 0xff};

void gp_write_pixels_1BPP_DB(void *start, uint8_t off,
                             size_t cnt, unsigned int val)
{
	int len = cnt;

	if (!len)
		return;

	/* Write start of the line */
	switch (off) {
	case 0:
	break;
	case 1:
		GP_SET_BITS1(1, 1, start, val);

		if (--len == 0)
			return;
		/* fallthrough */
	case 2:
		GP_SET_BITS1(2, 1, start, val);

		if (--len == 0)
			return;
		/* fallthrough */
	case 3:
		GP_SET_BITS1(3, 1, start, val);

		if (--len == 0)
			return;
		/* fallthrough */
	case 4:
		GP_SET_BITS1(4, 1, start, val);

		if (--len == 0)
			return;
		/* fallthrough */
	case 5:
		GP_SET_BITS1(5, 1, start, val);

		if (--len == 0)
			return;
		/* fallthrough */
	case 6:
		GP_SET_BITS1(6, 1, start, val);

		if (--len == 0)
			return;
		/* fallthrough */
	case 7:
		GP_SET_BITS1(7, 1, start, val);

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
		GP_SET_BITS1(6, 1, start, val);
		/* fallthrough */
	case 6:
		GP_SET_BITS1(5, 1, start, val);
		/* fallthrough */
	case 5:
		GP_SET_BITS1(4, 1, start, val);
		/* fallthrough */
	case 4:
		GP_SET_BITS1(3, 1, start, val);
		/* fallthrough */
	case 3:
		GP_SET_BITS1(2, 1, start, val);
		/* fallthrough */
	case 2:
		GP_SET_BITS1(1, 1, start, val);
		/* fallthrough */
	case 1:
		GP_SET_BITS1(0, 1, start, val);
	break;
	}
}

void gp_write_pixels_1BPP_UB(void *start, uint8_t off,
                             size_t cnt, unsigned int val)
{
	int len = cnt;

	if (!len)
		return;

	/* Write start of the line */
	switch (off) {
	case 0:
	break;
	case 1:
		GP_SET_BITS1(6, 1, start, val);

		if (--len == 0)
			return;
		/* fallthrough */
	case 2:
		GP_SET_BITS1(5, 1, start, val);

		if (--len == 0)
			return;
		/* fallthrough */
	case 3:
		GP_SET_BITS1(4, 1, start, val);

		if (--len == 0)
			return;
		/* fallthrough */
	case 4:
		GP_SET_BITS1(3, 1, start, val);

		if (--len == 0)
			return;
		/* fallthrough */
	case 5:
		GP_SET_BITS1(2, 1, start, val);

		if (--len == 0)
			return;
		/* fallthrough */
	case 6:
		GP_SET_BITS1(1, 1, start, val);

		if (--len == 0)
			return;
		/* fallthrough */
	case 7:
		GP_SET_BITS1(0, 1, start, val);

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
		GP_SET_BITS1(1, 1, start, val);
		/* fallthrough */
	case 6:
		GP_SET_BITS1(2, 1, start, val);
		/* fallthrough */
	case 5:
		GP_SET_BITS1(3, 1, start, val);
		/* fallthrough */
	case 4:
		GP_SET_BITS1(4, 1, start, val);
		/* fallthrough */
	case 3:
		GP_SET_BITS1(5, 1, start, val);
		/* fallthrough */
	case 2:
		GP_SET_BITS1(6, 1, start, val);
		/* fallthrough */
	case 1:
		GP_SET_BITS1(7, 1, start, val);
	break;
	}
}

static const uint8_t bytes_2BPP[] = {0x00, 0x55, 0xaa, 0xff};

void gp_write_pixels_2BPP_DB(void *start, uint8_t off,
                             size_t cnt, unsigned int val)
{
	int len = cnt;

	/* Write start of the line */
	switch (off) {
	case 0:
	break;
	case 1:
		GP_SET_BITS1(2, 2, start, val);

		if (--len == 0)
			return;
		/* fallthrough */
	case 2:
		GP_SET_BITS1(4, 2, start, val);

		if (--len == 0)
			return;
		/* fallthrough */
	case 3:
		GP_SET_BITS1(6, 2, start, val);

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
		GP_SET_BITS1(4, 2, start, val);
		/* fallthrough */
	case 2:
		GP_SET_BITS1(2, 2, start, val);
		/* fallthrough */
	case 1:
		GP_SET_BITS1(0, 2, start, val);
	break;
	}
}

void gp_write_pixels_2BPP_UB(void *start, uint8_t off,
                             size_t cnt, unsigned int val)
{
	int len = cnt;

	/* Write start of the line */
	switch (off) {
	case 0:
	break;
	case 1:
		GP_SET_BITS1(4, 2, start, val);

		if (--len == 0)
			return;
		/* fallthrough */
	case 2:
		GP_SET_BITS1(2, 2, start, val);

		if (--len == 0)
			return;
		/* fallthrough */
	case 3:
		GP_SET_BITS1(0, 2, start, val);

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
		GP_SET_BITS1(2, 2, start, val);
		/* fallthrough */
	case 2:
		GP_SET_BITS1(4, 2, start, val);
		/* fallthrough */
	case 1:
		GP_SET_BITS1(6, 2, start, val);
	break;
	}
}

static const uint8_t bytes_4BPP[] = {
	0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77,
	0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff
};

void gp_write_pixels_4BPP_DB(void *start, uint8_t off,
                            size_t cnt, unsigned int val)
{
	int len = cnt;

	/* Write start of the line */
	switch (off) {
	case 0:
	break;
	case 1:
		GP_SET_BITS1(4, 4, start, val);

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
		GP_SET_BITS1(0, 4, start, val);
	break;
	}
}

void gp_write_pixels_4BPP_UB(void *start, uint8_t off,
                             size_t cnt, unsigned int val)
{
	int len = cnt;

	/* Write start of the line */
	switch (off) {
	case 0:
	break;
	case 1:
		GP_SET_BITS1(0, 4, start, val);

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
		GP_SET_BITS1(4, 4, start, val);
	break;
	}
}

void gp_write_pixels_8BPP(void *start, size_t count, unsigned int value)
{
	memset(start, value, count);
}

void gp_write_pixels_16BPP(void *start, size_t count, unsigned int value)
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

void gp_write_pixels_16BPP_LE(void *start, size_t count, unsigned int value)
{
#if __BYTE_ORDER == __LITTLE_ENDIAN
	gp_write_pixels_16BPP(start, count, value);
#elif __BYTE_ORDER == __BIG_ENDIAN
	gp_write_pixels_16BPP(start, count, GP_SWAP_BYTES2(value));
#else
# error Unknown endianity!
#endif
}

void gp_write_pixels_16BPP_BE(void *start, size_t count, unsigned int value)
{
#if __BYTE_ORDER == __BIG_ENDIAN
	gp_write_pixels_16BPP(start, count, value);
#elif __BYTE_ORDER == __LITTLE_ENDIAN
	gp_write_pixels_16BPP(start, count, GP_SWAP_BYTES2(value));
#else
# error Unknown endianity!
#endif
}

void gp_write_pixels_24BPP(void *start, size_t count, unsigned int value)
{
	uint8_t *bytep = (uint8_t *) start;

	if (count == 0)
		return;

	/* How much bytes we are offset against the 32-bit boundary. */
	int shift = ((uintptr_t) bytep) % 4;

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

void gp_write_pixels_32BPP(void *start, size_t count, unsigned int value)
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
