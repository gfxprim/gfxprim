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
 * Copyright (C) 2009-2014 Cyril Hrubis <metan@ucw.cz>                       *
 *                                                                           *
 *****************************************************************************/

/*

  PNM portable bitmap header
  --------------------------

  Format:

  a magic number value of 'P' and one of
   '1' - PBM Bitmap ASCII
   '2' - PGM Gray   ASCII
   '3' - PPM RGB    ASCII
   '4' - PBM Bitmap Binary
   '5' - PGM Gray   Binary
   '6' - PPM RGB    Binary
  whitespace (blanks, TABs, CRs, LFs).
  ascii width
  whitespace
  ascii height
  whitespace
  maximal value (interval is 0 ... max) (not applicable for PBM)
  width * height ascii or binary values

  lines starting with '#' are comments to the end of line

 */

#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>
#include <errno.h>
#include <ctype.h>

#include <string.h>

#include "core/GP_Debug.h"
#include "core/GP_Context.h"
#include "core/GP_GetPutPixel.h"
#include "loaders/GP_LineConvert.h"

#include "loaders/GP_PNM.h"

struct pnm_header {
	char magic;
	uint32_t w;
	uint32_t h;
	uint32_t depth;
};

static char *pnm_names[] = {
	"ASCII encoded PBM",
	"ASCII encoded PGM",
	"ASCII encoded PPM",
	"Binary encoded PBM",
	"Binary encoded PGM",
	"Binary encoded PPM",
};

static const char *pnm_magic_name(char magic)
{
	return pnm_names[magic - '1'];
}

/*
 * P1 == ascii
 * P4 == rawbits
 */
static int is_bitmap(char magic)
{
	return magic == '1' || magic == '4';
}

int GP_MatchPBM(const void *buf)
{
	const char *b = buf;

	return b[0] == 'P' && is_bitmap(b[1]);
}

/*
 * P2 == ascii
 * P5 == rawbits
 */
static int is_graymap(char magic)
{
	return magic == '2' || magic == '5';
}

int GP_MatchPGM(const void *buf)
{
	const char *b = buf;

	return b[0] == 'P' && is_graymap(b[1]);
}

/*
 * P3 == ascii
 * P6 == rawbits
 */
static int is_pixmap(char magic)
{
	return magic == '3' || magic == '6';
}

int GP_MatchPPM(const void *buf)
{
	const char *b = buf;

	return b[0] == 'P' && is_pixmap(b[1]);
}

static int magic_is_valid(char magic)
{
	switch (magic) {
	case '1' ... '9':
		return 1;
	default:
		return 0;
	}
}

int GP_MatchPNM(const void *buf)
{
	const char *b = buf;

	return b[0] == 'P' && magic_is_valid(b[1]);
}

/*
 * Header parser
 */
enum state {
	S_START,
	S_COMMENT,
	S_INT,
};

/*
 * Simple buffer implementation on the top of the GP_IO
 */
struct buf {
	uint8_t buf[100];
	unsigned int buf_end;
	unsigned int buf_pos;
	GP_IO *io;
};

#define DECLARE_BUFFER(name, bio) \
	struct buf name = {.buf_end = 0, .buf_pos = 0, .io = bio}

static int getb(struct buf *buf)
{
	int ret;

	if (buf->buf_pos < buf->buf_end)
		return buf->buf[buf->buf_pos++];

	ret = GP_IORead(buf->io, buf->buf, sizeof(buf->buf));

	if (ret <= 0)
		return EOF;

	buf->buf_pos = 1;
	buf->buf_end = ret;

	return buf->buf[0];
}

static void ungetb(struct buf *buf, uint8_t byte)
{
	buf->buf[--buf->buf_pos] = byte;
}

static int fillb(struct buf *buf, void *ptr, size_t size)
{
	unsigned int buffered = buf->buf_end - buf->buf_pos;

	if (buffered) {
		unsigned int to_copy = GP_MIN(buffered, size);
		memcpy(ptr, buf->buf + buf->buf_pos, to_copy);
		buf->buf_pos += to_copy;
	}

	//TODO: refill buffer if request < 128
	if (size > buffered) {
		return GP_IOFill(buf->io, (char*)ptr + buffered,
		                 size - buffered);
	}

	return 0;
}

static int load_header(struct buf *buf, struct pnm_header *header)
{
	int h1, h2, c, state = S_START, val = 0, i = 0, err;

	h1 = getb(buf);
	h2 = getb(buf);

	if (h1 == EOF || h2 == EOF) {
		GP_DEBUG(1, "Failed to read header");
		return EIO;
	}

	if (h1 != 'P' || !magic_is_valid(h2)) {
		GP_DEBUG(1, "Invalid magic 0x%02x 0x%02x (%c %c)",
		         h1, h2, isprint(h1) ? h1 : ' ', isprint(h2) ? h2 : ' ');
		return EINVAL;
	}

	header->magic = h2;
	header->depth = 1;
	header->h = 0;
	header->w = 0;

	for (;;) {
		c = getb(buf);

		if (c == EOF) {
			err = EIO;
			goto err;
		}

		switch (state) {
		case S_START:
			switch (c) {
			case '#':
				state = S_COMMENT;
			break;
			case '0' ... '9':
				val = c - '0';
				state = S_INT;
			break;
			case '\n':
			case '\t':
			case ' ':
			case '\r':
			break;
			default:
				GP_WARN("Ignoring character 0x%02x (%c)",
				        c, isprint(c) ? c : ' ');
			}
		break;
		case S_COMMENT:
			if (c == '\n')
				state = S_START;
		break;
		case S_INT:
			switch (c) {
			case '0' ... '9':
				val *= 10;
				val += c - '0';
			break;
			default:
				switch (i++) {
				case 0:
					header->w = val;
				break;
				case 1:
					header->h = val;
					if (is_bitmap(header->magic))
						goto out;
				break;
				case 2:
					header->depth = val;
					goto out;
				}
				ungetb(buf, c);
				state = S_START;
			break;
			}
		break;
		}
	}

out:
	GP_DEBUG(1, "Have header P%c (%s) %"PRIu32"x%"PRIu32" depth=%"PRIu32,
	         header->magic, pnm_magic_name(header->magic),
		 header->w, header->h, header->depth);
	return 0;
err:
	GP_DEBUG(1, "Unexpected end of file when reading header");
	return err;
}

/*
 * ASCII data parser
 */
static int get_ascii_int(struct buf *buf, int *val)
{
	int c, in_number = 0;
	*val = 0;

	for (;;) {
		c = getb(buf);

		switch (c) {
		case EOF:
			if (!in_number) {
				GP_DEBUG(1, "Unexpected end of file");
				return EIO;
			}

			return 0;
		case '0' ... '9':
			*val *= 10;
			*val += c - '0';
			in_number = 1;
		break;
		case '\n':
		case '\t':
		case ' ':
		case '\r':
			if (in_number)
				return 0;
		break;
		default:
			if (in_number)
				return 0;
			else
				GP_WARN("Ignoring unexpected character 0x%02x %c",
				        c, isprint(c) ? c : ' ');
		}
	}
}

/*
 * Five times faster than printf("%u", byte)
 */
static inline int write_ascii_byte(GP_IO *io, uint8_t byte)
{
	if (byte >= 100)
		GP_IOPutC(io, '0' + byte/100);

	if (byte >= 10)
		GP_IOPutC(io, '0' + (byte%100)/10);

	GP_IOPutC(io, '0' + (byte%10));

	return GP_IOPutC(io, ' ');
}

/*
 * The PBM ASCII has the values inverted
 */
static int load_ascii_g1_inv(struct buf *buf, GP_Context *ctx,
                             GP_ProgressCallback *cb)
{
	uint32_t x, y;
	int val, err;

	for (y = 0; y < ctx->h; y++) {
		for (x = 0; x < ctx->w; x++) {

			if ((err = get_ascii_int(buf, &val)))
				return err;

			GP_PutPixel_Raw_1BPP_LE(ctx, x, y, !val);
		}

		if (GP_ProgressCallbackReport(cb, y, ctx->h, ctx->w)) {
			GP_DEBUG(1, "Operation aborted");
			return ECANCELED;
		}
	}

	GP_ProgressCallbackDone(cb);
	return 0;
}

//TODO: This is temporary till blit works with bitendian
#include "core/GP_BitSwap.h"

static int load_raw_g1_inv(struct buf *buf, GP_Context *ctx,
                           GP_ProgressCallback *cb)
{
	uint32_t x, y;
	uint8_t *addr;
	int val;

	for (y = 0; y < ctx->h; y++) {
		for (x = 0; x < ctx->w; x+=8) {

			if ((val = getb(buf)) == EOF)
				return EIO;

			addr = GP_PIXEL_ADDR(ctx, x, y);
			*addr = ~GP_BIT_SWAP_B1(val);
		}

		if (GP_ProgressCallbackReport(cb, y, ctx->h, ctx->w)) {
			GP_DEBUG(1, "Operation aborted");
			return ECANCELED;
		}
	}

	GP_ProgressCallbackDone(cb);
	return 0;
}

static int load_ascii_g1(struct buf *buf, GP_Context *ctx,
                         GP_ProgressCallback *cb)
{
	uint32_t x, y;
	int val, err;

	for (y = 0; y < ctx->h; y++) {
		for (x = 0; x < ctx->w; x++) {

			if ((err = get_ascii_int(buf, &val)))
				return err;

			if (val > 1) {
				GP_WARN("Value too large for 1BPP (%i)", val);
				val = 1;
			}

			GP_PutPixel_Raw_1BPP_LE(ctx, x, y, val);
		}

		if (GP_ProgressCallbackReport(cb, y, ctx->h, ctx->w)) {
			GP_DEBUG(1, "Operation aborted");
			return ECANCELED;
		}
	}

	GP_ProgressCallbackDone(cb);
	return 0;
}

static int load_ascii_g2(struct buf *buf, GP_Context *ctx,
                         GP_ProgressCallback *cb)
{
	uint32_t x, y;
	int val, err;

	for (y = 0; y < ctx->h; y++) {
		for (x = 0; x < ctx->w; x++) {

			if ((err = get_ascii_int(buf, &val)))
				return err;

			if (val > 3) {
				GP_WARN("Value too large for 2BPP (%i)", val);
				val = 3;
			}

			GP_PutPixel_Raw_2BPP_LE(ctx, x, y, val);
		}

		if (GP_ProgressCallbackReport(cb, y, ctx->h, ctx->w)) {
			GP_DEBUG(1, "Operation aborted");
			return ECANCELED;
		}
	}

	GP_ProgressCallbackDone(cb);
	return 0;
}

static int load_ascii_g4(struct buf *buf, GP_Context *ctx,
                         GP_ProgressCallback *cb)
{
	uint32_t x, y;
	int val, err;

	for (y = 0; y < ctx->h; y++) {
		for (x = 0; x < ctx->w; x++) {

			if ((err = get_ascii_int(buf, &val)))
				return err;

			if (val > 15) {
				GP_WARN("Value too large for 4BPP (%i)", val);
				val = 15;
			}

			GP_PutPixel_Raw_4BPP_LE(ctx, x, y, val);
		}

		if (GP_ProgressCallbackReport(cb, y, ctx->h, ctx->w)) {
			GP_DEBUG(1, "Operation aborted");
			return ECANCELED;
		}
	}

	GP_ProgressCallbackDone(cb);
	return 0;
}

static int load_ascii_g8(struct buf *buf, GP_Context *ctx,
                         GP_ProgressCallback *cb)
{
	uint32_t x, y;
	int val, err;

	for (y = 0; y < ctx->h; y++) {
		for (x = 0; x < ctx->w; x++) {

			if ((err = get_ascii_int(buf, &val)))
				return err;

			if (val > 255) {
				GP_WARN("Value too large for 8BPP (%i)", val);
				val = 255;
			}

			GP_PutPixel_Raw_8BPP(ctx, x, y, val);
		}

		if (GP_ProgressCallbackReport(cb, y, ctx->h, ctx->w)) {
			GP_DEBUG(1, "Operation aborted");
			return ECANCELED;
		}
	}

	GP_ProgressCallbackDone(cb);
	return 0;
}

static int load_bin_g8(struct buf *buf, GP_Context *ctx,
                       GP_ProgressCallback *cb)
{
	uint32_t y;

	for (y = 0; y < ctx->h; y++) {
		uint8_t *addr = GP_PIXEL_ADDR(ctx, 0, y);

		if (fillb(buf, addr, ctx->w))
			return errno;

		if (GP_ProgressCallbackReport(cb, y, ctx->h, ctx->w)) {
			GP_DEBUG(1, "Operation aborted");
			return ECANCELED;
		}
	}

	GP_ProgressCallbackDone(cb);
	return 0;
}

static int load_ascii_rgb888(struct buf *buf, GP_Context *ctx,
                             GP_ProgressCallback *cb)
{
	uint32_t x, y;
	int r, g, b, err;

	for (y = 0; y < ctx->h; y++) {
		for (x = 0; x < ctx->w; x++) {

			if ((err = get_ascii_int(buf, &r)))
				return err;

			if (r > 255) {
				GP_WARN("R value too large (%i)", r);
				r = 255;
			}

			if ((err = get_ascii_int(buf, &g)))
				return err;

			if (g > 255) {
				GP_WARN("G value too large (%i)", r);
				g = 255;
			}

			if ((err = get_ascii_int(buf, &b)))
				return err;

			if (b > 255) {
				GP_WARN("G value too large (%i)", r);
				b = 255;
			}

			GP_PutPixel_Raw_24BPP(ctx, x, y,
			                      GP_Pixel_CREATE_RGB888(r, g, b));
		}

		if (GP_ProgressCallbackReport(cb, y, ctx->h, ctx->w)) {
			GP_DEBUG(1, "Operation aborted");
			return ECANCELED;
		}
	}

	GP_ProgressCallbackDone(cb);
	return 0;
}

static int load_bin_rgb888(struct buf *buf, GP_Context *ctx,
                           GP_ProgressCallback *cb)
{
	uint32_t y, x;

	for (y = 0; y < ctx->h; y++) {
		uint8_t *addr = GP_PIXEL_ADDR(ctx, 0, y);

		if (fillb(buf, addr, ctx->w * 3))
			return errno;

		for (x = 0; x < ctx->w; x++)
			GP_SWAP(addr[3*x], addr[3*x + 2]);

		if (GP_ProgressCallbackReport(cb, y, ctx->h, ctx->w)) {
			GP_DEBUG(1, "Operation aborted");
			return ECANCELED;
		}
	}

	GP_ProgressCallbackDone(cb);
	return 0;
}

static int save_ascii(GP_IO *io, const GP_Context *ctx,
                      GP_ProgressCallback *cb, int inv)
{
	uint32_t x, y;
	int err;

	for (y = 0; y < ctx->h; y++) {
		for (x = 0; x < ctx->w; x++) {
			int val = GP_GetPixel_Raw(ctx, x, y);

			if (inv)
				val = !val;

			if (write_ascii_byte(io, val)) {
				err = errno;
				GP_DEBUG(1, "Failed to write data");
				return err;
			}

		}
		if (GP_ProgressCallbackReport(cb, y, ctx->h, ctx->w)) {
			GP_DEBUG(1, "Operation aborted");
			return ECANCELED;
		}

		if (GP_IOPutC(io, '\n'))
			return errno;
	}

	GP_ProgressCallbackDone(cb);
	return 0;
}

static int read_bitmap(struct buf *buf, struct pnm_header *header,
                       GP_Context **img, GP_ProgressCallback *callback)
{
	GP_Context *ret;
	int err;

	if (!is_bitmap(header->magic)) {
		GP_DEBUG(1, "Invalid Bitmap magic P%c", header->magic);
		err = EINVAL;
		goto err0;
	}

	ret = GP_ContextAlloc(header->w, header->h, GP_PIXEL_G1);

	if (ret == NULL) {
		err = ENOMEM;
		goto err1;
	}

	if (header->magic == '1')
		err = load_ascii_g1_inv(buf, ret, callback);
	else
		err = load_raw_g1_inv(buf, ret, callback);

	if (err)
		goto err1;

	*img = ret;
	return 0;
err1:
	GP_ContextFree(ret);
err0:
	errno = err;
	return 1;
}

int GP_ReadPBMEx(GP_IO *io, GP_Context **img, GP_DataStorage *storage,
                 GP_ProgressCallback *callback)
{
	struct pnm_header header;
	DECLARE_BUFFER(buf, io);
	int err;

	(void) storage;

	err = load_header(&buf, &header);
	if (err) {
		errno = err;
		return 1;
	}

	if (!img)
		return 0;

	return read_bitmap(&buf, &header, img, callback);
}

static GP_PixelType pbm_save_pixels[] = {
	GP_PIXEL_G1,
	GP_PIXEL_UNKNOWN,
};

int GP_WritePBM(const GP_Context *src, GP_IO *io,
                GP_ProgressCallback *callback)
{
	GP_IO *bio;
	int err;

	GP_DEBUG(1, "Writing PBM into I/O (%p)", io);

	if (src->pixel_type != GP_PIXEL_G1) {
		GP_DEBUG(1, "Invalid pixel type '%s'",
		         GP_PixelTypeName(src->pixel_type));
		errno = EINVAL;
		return 1;
	}

	bio = GP_IOWBuffer(io, 0);
	if (!bio)
		return 1;

	if (GP_IOPrintF(io, "P1\n%u %u\n",
	            (unsigned int) src->w, (unsigned int) src->h)) {
		err = errno;
		goto err;
	}

	if ((err = save_ascii(bio, src, callback, 1)))
		goto err;

	return GP_IOClose(bio);
err:
	GP_IOClose(bio);
	errno = err;
	return 1;
}

static GP_Pixel depth_to_pixel(int depth)
{
	switch (depth) {
	case 1:
		return GP_PIXEL_G1;
	case 3:
		return GP_PIXEL_G2;
	case 15:
		return GP_PIXEL_G4;
	case 255:
		return GP_PIXEL_G8;
	default:
		return GP_PIXEL_UNKNOWN;
	}
}

static int load_ascii_graymap(struct buf *buf, struct pnm_header *header,
                              GP_Context *ret, GP_ProgressCallback *callback)
{
	int err = ENOSYS;

	switch (header->depth) {
	case 1:
		err = load_ascii_g1(buf, ret, callback);
	break;
	case 3:
		err = load_ascii_g2(buf, ret, callback);
	break;
	case 15:
		err = load_ascii_g4(buf, ret, callback);
	break;
	case 255:
		err = load_ascii_g8(buf, ret, callback);
	break;
	}

	return err;
}

static int load_bin_graymap(struct buf *buf, struct pnm_header *header,
                            GP_Context *ret, GP_ProgressCallback *callback)
{
	int err = ENOSYS;

	switch (header->depth) {
	case 255:
		err = load_bin_g8(buf, ret, callback);
	break;
	}

	return err;
}

static int read_graymap(struct buf *buf, struct pnm_header *header,
                        GP_Context **img, GP_ProgressCallback *callback)
{
	GP_Context *ret;
	GP_PixelType pixel_type;
	int err;

	if (!is_graymap(header->magic)) {
		GP_DEBUG(1, "Invalid graymap magic P%c", header->magic);
		err = EINVAL;
		goto err0;
	}

	if ((pixel_type = depth_to_pixel(header->depth)) == GP_PIXEL_UNKNOWN) {
		GP_DEBUG(1, "Invalid number of grays %u", header->depth);
		err = EINVAL;
		goto err0;
	}

	ret = GP_ContextAlloc(header->w, header->h, pixel_type);

	if (ret == NULL) {
		err = ENOMEM;
		goto err1;
	}

	if (header->magic == '5')
		err = load_bin_graymap(buf, header, ret, callback);
	else
		err = load_ascii_graymap(buf, header, ret, callback);

	if (err)
		goto err1;

	*img = ret;
	return 0;
err1:
	GP_ContextFree(ret);
err0:
	errno = err;
	return 1;
}

int GP_ReadPGMEx(GP_IO *io, GP_Context **img, GP_DataStorage *storage,
                 GP_ProgressCallback *callback)
{
	struct pnm_header header;
	DECLARE_BUFFER(buf, io);
	int err;

	(void) storage;

	err = load_header(&buf, &header);
	if (err) {
		errno = err;
		return 1;
	}

	if (!img)
		return 0;

	return read_graymap(&buf, &header, img, callback);
}

static int pixel_to_depth(GP_Pixel pixel)
{
	switch (pixel) {
	case GP_PIXEL_G1:
		return 1;
	case GP_PIXEL_G2:
		return 3;
	case GP_PIXEL_G4:
		return 15;
	case GP_PIXEL_G8:
		return 255;
	default:
		return -1;
	}
}

static GP_PixelType pgm_save_pixels[] = {
	GP_PIXEL_G1,
	GP_PIXEL_G2,
	GP_PIXEL_G4,
	GP_PIXEL_G8,
	GP_PIXEL_UNKNOWN,
};

int GP_WritePGM(const GP_Context *src, GP_IO *io,
                GP_ProgressCallback *callback)
{
	int err, depth;
	GP_IO *bio;

	GP_DEBUG(1, "Writing PGM to I/O (%p)", io);

	if ((depth = pixel_to_depth(src->pixel_type)) == -1) {
		GP_DEBUG(1, "Invalid pixel type '%s'",
		         GP_PixelTypeName(src->pixel_type));
		errno = EINVAL;
		return 1;
	}

	bio = GP_IOWBuffer(io, 0);
	if (!bio)
		return 1;

	if (GP_IOPrintF(io, "P2\n%u %u\n%u\n",
	            (unsigned int) src->w, (unsigned int) src->h, depth)) {
		err = errno;
		goto err;
	}

	if ((err = save_ascii(bio, src, callback, 0)))
		goto err;

	return GP_IOClose(bio);
err:
	GP_IOClose(bio);
	errno = err;
	return 1;
}

static int read_pixmap(struct buf *buf, struct pnm_header *header,
                       GP_Context **img, GP_ProgressCallback *callback)
{
	GP_Context *ret;
	int err = 0;

	if (!is_pixmap(header->magic)) {
		GP_DEBUG(1, "Invalid Pixmap magic P%c", header->magic);
		err = EINVAL;
		goto err0;
	}

	if (header->depth != 255) {
		GP_DEBUG(1, "Unsupported depth %"PRIu32, header->depth);
		err = ENOSYS;
		goto err0;
	}

	ret = GP_ContextAlloc(header->w, header->h, GP_PIXEL_RGB888);

	if (ret == NULL) {
		err = ENOMEM;
		goto err0;
	}

	switch (header->magic) {
	case '3':
		err = load_ascii_rgb888(buf, ret, callback);
	break;
	case '6':
		err = load_bin_rgb888(buf, ret, callback);
	break;
	}

	if (err)
		goto err1;

	*img = ret;
	return 0;
err1:
	GP_ContextFree(ret);
err0:
	errno = err;
	return 1;
}

int GP_ReadPPMEx(GP_IO *io, GP_Context **img, GP_DataStorage *storage,
                 GP_ProgressCallback *callback)
{
	struct pnm_header header;
	DECLARE_BUFFER(buf, io);
	int err;

	(void) storage;

	err = load_header(&buf, &header);
	if (err) {
		errno = err;
		return 1;
	}

	if (!img)
		return 0;

	return read_pixmap(&buf, &header, img, callback);
}

static int write_binary_ppm(FILE *f, GP_Context *src)
{
	uint32_t x, y;

	for (y = 0; y < src->h; y++)
		for (x = 0; x < src->w; x++) {
			GP_Pixel pix = GP_GetPixel_Raw_24BPP(src, x, y);

			uint8_t buf[3] = {GP_Pixel_GET_R_RGB888(pix),
			                  GP_Pixel_GET_G_RGB888(pix),
			                  GP_Pixel_GET_B_RGB888(pix)};

			if (fwrite(buf, 3, 1, f) < 1)
				return 1;
		}

	return 0;
}

static int save_ascii_rgb888(GP_IO *io, const GP_Context *ctx,
                             GP_LineConvert Convert, GP_ProgressCallback *cb)
{
	uint32_t x, y;
	int ret = 0;
	uint8_t buf[3 * ctx->w], *addr;

	for (y = 0; y < ctx->h; y++) {

		addr = GP_PIXEL_ADDR(ctx, 0, y);

		if (Convert) {
			Convert(addr, buf, ctx->w);
			addr = buf;
		}

		for (x = 0; x < ctx->w; x++) {
			ret |= write_ascii_byte(io, addr[2]);
			ret |= write_ascii_byte(io, addr[1]);
			ret |= write_ascii_byte(io, addr[0]);

			if (ret)
				return errno;

			addr+=3;
		}

		if (GP_ProgressCallbackReport(cb, y, ctx->h, ctx->w)) {
			GP_DEBUG(1, "Operation aborted");
			return ECANCELED;
		}

		if (GP_IOPutC(io, '\n'))
			return errno;
	}

	GP_ProgressCallbackDone(cb);
	return 0;
}

static GP_PixelType ppm_save_pixels[] = {
	GP_PIXEL_RGB888,
	GP_PIXEL_UNKNOWN,
};

int GP_WritePPM(const GP_Context *src, GP_IO *io,
                GP_ProgressCallback *callback)
{
	GP_Pixel out_pix;
	GP_LineConvert Convert;
	GP_IO *bio;
	int err = 0;

	GP_DEBUG(1, "Writing PPM into I/O (%p)", io);

	out_pix = GP_LineConvertible(src->pixel_type, ppm_save_pixels);

	if (out_pix == GP_PIXEL_UNKNOWN) {
		GP_DEBUG(1, "Invalid pixel type '%s'",
		         GP_PixelTypeName(src->pixel_type));
		errno = EINVAL;
		return 1;
	}

	bio = GP_IOWBuffer(io, 0);
	if (!bio)
		return 1;

	if (GP_IOPrintF(io, "P3\n%u %u\n255\n",
	            (unsigned int) src->w, (unsigned int) src->h)) {
		err = errno;
		goto err;
	}

	Convert = GP_LineConvertGet(src->pixel_type, out_pix);

	if ((err = save_ascii_rgb888(bio, src, Convert, callback)))
		goto err;

	return GP_IOClose(bio);
err:
	GP_IOClose(bio);
	errno = err;
	return 1;
}

int GP_ReadPNMEx(GP_IO *io, GP_Context **img, GP_DataStorage *storage,
                 GP_ProgressCallback *callback)
{
	struct pnm_header header;
	DECLARE_BUFFER(buf, io);
	int err, ret = 1;

	(void) storage;

	err = load_header(&buf, &header);
	if (err) {
		errno = err;
		return 1;
	}

	if (!img)
		return 0;

	if (is_bitmap(header.magic))
		ret = read_bitmap(&buf, &header, img, callback);

	if (is_graymap(header.magic))
		ret = read_graymap(&buf, &header, img, callback);

	if (is_pixmap(header.magic))
		ret = read_pixmap(&buf, &header, img, callback);

	return ret;
}

static GP_PixelType pnm_save_pixels[] = {
	GP_PIXEL_G1,
	GP_PIXEL_G2,
	GP_PIXEL_G4,
	GP_PIXEL_G8,
	GP_PIXEL_RGB888,
	GP_PIXEL_UNKNOWN,
};

int GP_WritePNM(const GP_Context *src, GP_IO *io,
                GP_ProgressCallback *callback)
{
	switch (src->pixel_type) {
	case GP_PIXEL_G1:
	case GP_PIXEL_G2:
	case GP_PIXEL_G4:
	case GP_PIXEL_G8:
		return GP_WritePGM(src, io, callback);
	case GP_PIXEL_RGB888:
		return GP_WritePPM(src, io, callback);
	default:
		if (GP_LineConvertible(src->pixel_type, ppm_save_pixels))
			return GP_WritePPM(src, io, callback);

		errno = EINVAL;
		return 1;
	}
}

GP_Context *GP_ReadPBM(GP_IO *io, GP_ProgressCallback *callback)
{
	return GP_LoaderReadImage(&GP_PBM, io, callback);
}

GP_Context *GP_LoadPBM(const char *src_path, GP_ProgressCallback *callback)
{
	return GP_LoaderLoadImage(&GP_PBM, src_path, callback);
}

int GP_SavePBM(const GP_Context *src, const char *dst_path,
               GP_ProgressCallback *callback)
{
	return GP_LoaderSaveImage(&GP_PBM, src, dst_path, callback);
}

GP_Context *GP_ReadPGM(GP_IO *io, GP_ProgressCallback *callback)
{
	return GP_LoaderReadImage(&GP_PGM, io, callback);
}

GP_Context *GP_LoadPGM(const char *src_path, GP_ProgressCallback *callback)
{
	return GP_LoaderLoadImage(&GP_PGM, src_path, callback);
}

int GP_SavePGM(const GP_Context *src, const char *dst_path,
               GP_ProgressCallback *callback)
{
	return GP_LoaderSaveImage(&GP_PGM, src, dst_path, callback);
}

GP_Context *GP_ReadPPM(GP_IO *io, GP_ProgressCallback *callback)
{
	return GP_LoaderReadImage(&GP_PPM, io, callback);
}

GP_Context *GP_LoadPPM(const char *src_path, GP_ProgressCallback *callback)
{
	return GP_LoaderLoadImage(&GP_PPM, src_path, callback);
}

int GP_SavePPM(const GP_Context *src, const char *dst_path,
               GP_ProgressCallback *callback)
{
	return GP_LoaderSaveImage(&GP_PPM, src, dst_path, callback);
}

GP_Context *GP_ReadPNM(GP_IO *io, GP_ProgressCallback *callback)
{
	return GP_LoaderReadImage(&GP_PNM, io, callback);
}

GP_Context *GP_LoadPNM(const char *src_path, GP_ProgressCallback *callback)
{
	return GP_LoaderLoadImage(&GP_PNM, src_path, callback);
}

int GP_SavePNM(const GP_Context *src, const char *dst_path,
               GP_ProgressCallback *callback)
{
	return GP_LoaderSaveImage(&GP_PNM, src, dst_path, callback);
}

struct GP_Loader GP_PBM = {
	.Read = GP_ReadPBMEx,
	.Write = GP_WritePBM,
	.save_ptypes = pbm_save_pixels,
	.Match = GP_MatchPBM,

	.fmt_name = "Netpbm portable Bitmap",
	.extensions = {"pbm", NULL},
};

struct GP_Loader GP_PGM = {
	.Read = GP_ReadPGMEx,
	.Write = GP_WritePGM,
	.save_ptypes = pgm_save_pixels,
	.Match = GP_MatchPGM,

	.fmt_name = "Netpbm portable Graymap",
	.extensions = {"pgm", NULL},
};

struct GP_Loader GP_PPM = {
	.Read = GP_ReadPPMEx,
	.Write = GP_WritePPM,
	.save_ptypes = ppm_save_pixels,
	.Match = GP_MatchPPM,

	.fmt_name = "Netpbm portable Pixmap",
	.extensions = {"ppm", NULL},
};

struct GP_Loader GP_PNM = {
	.Read = GP_ReadPNMEx,
	.Write = GP_WritePNM,
	.save_ptypes = pnm_save_pixels,
	/*
	 * Avoid double Match
	 * This format is covered by PBM, PGM and PPM
	 */
	.Match = NULL,

	.fmt_name = "Netpbm portable Anymap",
	.extensions = {"pnm", NULL},
};
