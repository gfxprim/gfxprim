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
 * Copyright (C) 2009-2013 Cyril Hrubis <metan@ucw.cz>                       *
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

static int load_header(FILE *f, struct pnm_header *header)
{
	int h1, h2, c, state = S_START, val = 0, i = 0, err;

	h1 = fgetc(f);
	h2 = fgetc(f);

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
		c = fgetc(f);

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
				ungetc(c, f);
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
static int get_ascii_int(FILE *f, int *val)
{
	int c, in_number = 0;
	*val = 0;

	for (;;) {
		c = getc_unlocked(f);

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
static inline int write_ascii_byte(FILE *f, uint8_t byte)
{
	if (byte >= 100)
		fputc_unlocked('0' + byte/100, f);

	if (byte >= 10)
		fputc_unlocked('0' + (byte%100)/10, f);

	fputc_unlocked('0' + (byte%10), f);

	return fputc_unlocked(' ', f) == EOF;
}

/*
 * The PBM ASCII has the values inverted
 */
static int load_ascii_g1_inv(FILE *f, GP_Context *ctx, GP_ProgressCallback *cb)
{
	uint32_t x, y;
	int val, err;

	for (y = 0; y < ctx->h; y++) {
		for (x = 0; x < ctx->w; x++) {

			if ((err = get_ascii_int(f, &val)))
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

static int load_raw_g1_inv(FILE *f, GP_Context *ctx, GP_ProgressCallback *cb)
{
	uint32_t x, y;
	uint8_t *addr;
	int val;

	for (y = 0; y < ctx->h; y++) {
		for (x = 0; x < ctx->w; x+=8) {

			if ((val = fgetc(f)) == EOF)
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

static int load_ascii_g1(FILE *f, GP_Context *ctx, GP_ProgressCallback *cb)
{
	uint32_t x, y;
	int val, err;

	for (y = 0; y < ctx->h; y++) {
		for (x = 0; x < ctx->w; x++) {

			if ((err = get_ascii_int(f, &val)))
				return err;

			if (val > 1) {
				GP_WARN("Value too large for 2BPP (%i)", val);
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

static int load_ascii_g2(FILE *f, GP_Context *ctx, GP_ProgressCallback *cb)
{
	uint32_t x, y;
	int val, err;

	for (y = 0; y < ctx->h; y++) {
		for (x = 0; x < ctx->w; x++) {

			if ((err = get_ascii_int(f, &val)))
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

static int load_ascii_g4(FILE *f, GP_Context *ctx, GP_ProgressCallback *cb)
{
	uint32_t x, y;
	int val, err;

	for (y = 0; y < ctx->h; y++) {
		for (x = 0; x < ctx->w; x++) {

			if ((err = get_ascii_int(f, &val)))
				return err;

			if (val > 15) {
				GP_WARN("Value too large for 4BPP (%i)", val);
				val = 3;
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

static int load_ascii_g8(FILE *f, GP_Context *ctx, GP_ProgressCallback *cb)
{
	uint32_t x, y;
	int val, err;

	for (y = 0; y < ctx->h; y++) {
		for (x = 0; x < ctx->w; x++) {

			if ((err = get_ascii_int(f, &val)))
				return err;

			if (val > 15) {
				GP_WARN("Value too large for 4BPP (%i)", val);
				val = 3;
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

static int load_bin_g8(FILE *f, GP_Context *ctx, GP_ProgressCallback *cb)
{
	uint32_t y;

	for (y = 0; y < ctx->h; y++) {
		uint8_t *addr = GP_PIXEL_ADDR(ctx, 0, y);

		if (fread(addr, ctx->w, 1, f) != 1)
			return errno;

		if (GP_ProgressCallbackReport(cb, y, ctx->h, ctx->w)) {
			GP_DEBUG(1, "Operation aborted");
			return ECANCELED;
		}
	}

	GP_ProgressCallbackDone(cb);
	return 0;
}

static int load_ascii_rgb888(FILE *f, GP_Context *ctx, GP_ProgressCallback *cb)
{
	uint32_t x, y;
	int r, g, b, err;

	for (y = 0; y < ctx->h; y++) {
		for (x = 0; x < ctx->w; x++) {

			if ((err = get_ascii_int(f, &r)))
				return err;

			if (r > 255) {
				GP_WARN("R value too large (%i)", r);
				r = 255;
			}

			if ((err = get_ascii_int(f, &g)))
				return err;

			if (g > 255) {
				GP_WARN("G value too large (%i)", r);
				g = 255;
			}

			if ((err = get_ascii_int(f, &b)))
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

static int load_bin_rgb888(FILE *f, GP_Context *ctx, GP_ProgressCallback *cb)
{
	uint32_t y, x;

	for (y = 0; y < ctx->h; y++) {
		uint8_t *addr = GP_PIXEL_ADDR(ctx, 0, y);

		if (fread(addr, ctx->w * 3, 1, f) != 1)
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

static int save_ascii(FILE *f, const GP_Context *ctx,
                      GP_ProgressCallback *cb, int inv)
{
	uint32_t x, y;
	int err;

	for (y = 0; y < ctx->h; y++) {
		for (x = 0; x < ctx->w; x++) {
			int val = GP_GetPixel_Raw(ctx, x, y);

			if (inv)
				val = !val;

			if (write_ascii_byte(f, val)) {
				err = errno;
				GP_DEBUG(1, "Failed to write data");
				return err;
			}

		}
		if (GP_ProgressCallbackReport(cb, y, ctx->h, ctx->w)) {
			GP_DEBUG(1, "Operation aborted");
			return ECANCELED;
		}
		fprintf(f, "\n");
	}

	GP_ProgressCallbackDone(cb);
	return 0;
}

static FILE *read_header(const char *src_path, struct pnm_header *header)
{
	FILE *f;
	int err;

	f = fopen(src_path, "r");

	if (f == NULL)
		return NULL;

	if ((err = load_header(f, header))) {
		fclose(f);
		errno = err;
		return NULL;
	}

	return f;
}

static GP_Context *read_bitmap(FILE *f, struct pnm_header *header, int flag,
                               GP_ProgressCallback *callback)
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
		err = load_ascii_g1_inv(f, ret, callback);
	else
		err = load_raw_g1_inv(f, ret, callback);

	if (err)
		goto err1;

	if (flag)
		fclose(f);

	return ret;
err1:
	GP_ContextFree(ret);

	if (flag)
		fclose(f);
err0:
	errno = err;
	return NULL;
}

GP_Context *GP_LoadPBM(const char *src_path, GP_ProgressCallback *callback)
{
	struct pnm_header header;
	FILE *f;

	f = read_header(src_path, &header);

	if (f == NULL)
		return NULL;

	return read_bitmap(f, &header, 1, callback);
}

int GP_SavePBM(const GP_Context *src, const char *dst_path,
               GP_ProgressCallback *callback)
{
	FILE *f;
	int err;

	GP_DEBUG(1, "Saving context %ux%u %s to '%s'",
	         src->w, src->h, GP_PixelTypeName(src->pixel_type), dst_path);

	if (src->pixel_type != GP_PIXEL_G1) {
		GP_DEBUG(1, "Invalid pixel type '%s'",
		         GP_PixelTypeName(src->pixel_type));
		errno = EINVAL;
		return 1;
	}

	f = fopen(dst_path, "w");

	if (f == NULL)
		return 1;

	if (fprintf(f, "P1\n%u %u\n",
	            (unsigned int) src->w, (unsigned int) src->h) < 0) {
		err = EIO;
		goto err0;
	}

	if ((err = save_ascii(f, src, callback, 1)))
		goto err0;

	if (fclose(f))
		goto err0;

	return 0;
err0:
	fclose(f);
	unlink(dst_path);
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

static int load_ascii_graymap(FILE *f, struct pnm_header *header,
                              GP_Context *ret, GP_ProgressCallback *callback)
{
	int err = ENOSYS;

	switch (header->depth) {
	case 1:
		err = load_ascii_g1(f, ret, callback);
	break;
	case 3:
		err = load_ascii_g2(f, ret, callback);
	break;
	case 15:
		err = load_ascii_g4(f, ret, callback);
	break;
	case 255:
		err = load_ascii_g8(f, ret, callback);
	break;
	}

	return err;
}

static int load_bin_graymap(FILE *f, struct pnm_header *header,
                            GP_Context *ret, GP_ProgressCallback *callback)
{
	int err = ENOSYS;

	switch (header->depth) {
	case 255:
		err = load_bin_g8(f, ret, callback);
	break;
	}

	return err;
}

static GP_Context *read_graymap(FILE *f, struct pnm_header *header, int flag,
                                GP_ProgressCallback *callback)
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
		err = load_bin_graymap(f, header, ret, callback);
	else
		err = load_ascii_graymap(f, header, ret, callback);

	if (err)
		goto err1;

	if (flag)
		fclose(f);

	return ret;
err1:
	GP_ContextFree(ret);

	if (flag)
		fclose(f);
err0:
	errno = err;
	return NULL;
}

GP_Context *GP_LoadPGM(const char *src_path, GP_ProgressCallback *callback)
{
	struct pnm_header header;
	FILE *f;

	f = read_header(src_path, &header);

	if (f == NULL)
		return NULL;

	return read_graymap(f, &header, 1, callback);
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

int GP_SavePGM(const GP_Context *src, const char *dst_path,
               GP_ProgressCallback *callback)
{
	FILE *f;
	int depth;
	int err = EIO;

	GP_DEBUG(1, "Saving context %ux%u %s to '%s'",
	         src->w, src->h, GP_PixelTypeName(src->pixel_type), dst_path);

	if ((depth = pixel_to_depth(src->pixel_type)) == -1) {
		GP_DEBUG(1, "Invalid pixel type '%s'",
		         GP_PixelTypeName(src->pixel_type));
		errno = EINVAL;
		return 1;
	}

	f = fopen(dst_path, "w");

	if (f == NULL) {
		err = errno;
		GP_DEBUG(1, "Failed to open file '%s': %s",
		         dst_path, strerror(errno));
		goto err0;
	}

	if (fprintf(f, "P2\n%u %u\n%u\n",
	            (unsigned int) src->w, (unsigned int) src->h, depth) < 0)
		goto err1;

	if ((err = save_ascii(f, src, callback, 0)))
		goto err1;

	if (fclose(f)) {
		err = errno;
		GP_DEBUG(1, "Failed to close file '%s': %s",
		         dst_path, strerror(errno));
		goto err0;
	}

	return 0;
err1:
	fclose(f);
err0:
	unlink(dst_path);
	errno = err;
	return 1;
}

static GP_Context *read_pixmap(FILE *f, struct pnm_header *header, int flag,
                                GP_ProgressCallback *callback)
{
	GP_Context *ret;
	int err;

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
		if ((err = load_ascii_rgb888(f, ret, callback)))
			goto err1;
	break;
	case '6':
		if ((err = load_bin_rgb888(f, ret, callback)))
			goto err1;
	break;
	}

	if (flag)
		fclose(f);

	return ret;
err1:
	GP_ContextFree(ret);

	if (flag)
		fclose(f);
err0:
	errno = err;
	return NULL;
}

GP_Context *GP_LoadPPM(const char *src_path, GP_ProgressCallback *callback)
{
	struct pnm_header header;
	FILE *f;

	f = read_header(src_path, &header);

	if (f == NULL)
		return NULL;

	return read_pixmap(f, &header, 1, callback);
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

static int save_ascii_rgb888(FILE *f, const GP_Context *ctx,
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
			addr+=3;

			ret |= write_ascii_byte(f, addr[2]);
			ret |= write_ascii_byte(f, addr[1]);
			ret |= write_ascii_byte(f, addr[0]);

			if (ret)
				return errno;
		}

		if (GP_ProgressCallbackReport(cb, y, ctx->h, ctx->w)) {
			GP_DEBUG(1, "Operation aborted");
			return ECANCELED;
		}

		if (fprintf(f, "\n") < 0)
			return errno;
	}

	GP_ProgressCallbackDone(cb);
	return 0;
}

static GP_PixelType ppm_save_pixels[] = {
	GP_PIXEL_RGB888,
	GP_PIXEL_UNKNOWN,
};

int GP_SavePPM(const GP_Context *src, const char *dst_path,
               GP_ProgressCallback *callback)
{
	GP_Pixel out_pix;
	GP_LineConvert Convert;
	FILE *f;
	int err = EIO;

	GP_DEBUG(1, "Saving context %ux%u %s to '%s'",
	         src->w, src->h, GP_PixelTypeName(src->pixel_type), dst_path);

	out_pix = GP_LineConvertible(src->pixel_type, ppm_save_pixels);

	if (out_pix == GP_PIXEL_UNKNOWN) {
		GP_DEBUG(1, "Invalid pixel type '%s'",
		         GP_PixelTypeName(src->pixel_type));
		errno = EINVAL;
		return 1;
	}

	f = fopen(dst_path, "w");

	if (f == NULL) {
		err = errno;
		GP_DEBUG(1, "Failed to open file '%s': %s",
		         dst_path, strerror(errno));
		goto err0;
	}

	if (fprintf(f, "P3\n%u %u\n255\n",
	            (unsigned int) src->w, (unsigned int) src->h) < 0)
		goto err1;

	Convert = GP_LineConvertGet(src->pixel_type, out_pix);

	if ((err = save_ascii_rgb888(f, src, Convert, callback)))
		goto err1;

	if (fclose(f)) {
		err = errno;
		GP_DEBUG(1, "Failed to close file '%s': %s",
		         dst_path, strerror(errno));
		goto err0;
	}

	return 0;
err1:
	fclose(f);
err0:
	errno = err;
	unlink(dst_path);
	return 1;
}

GP_Context *GP_LoadPNM(const char *src_path, GP_ProgressCallback *callback)
{
	FILE *f;
	GP_Context *ret = NULL;
	struct pnm_header header;
	int err;

	f = fopen(src_path, "r");

	if (f == NULL) {
		err = errno;
		GP_DEBUG(1, "Failed to open file '%s': %s",
		         src_path, strerror(errno));
		goto err0;
	}

	if ((err = load_header(f, &header)))
		goto err1;

	if (is_bitmap(header.magic))
		ret = read_bitmap(f, &header, 1, callback);

	if (is_graymap(header.magic))
		ret = read_graymap(f, &header, 1, callback);

	if (is_pixmap(header.magic))
		ret = read_pixmap(f, &header, 1, callback);

	return ret;
err1:
	fclose(f);
err0:
	errno = err;
	return NULL;
}

int GP_SavePNM(const GP_Context *src, const char *dst_path,
               GP_ProgressCallback *callback)
{
	switch (src->pixel_type) {
	case GP_PIXEL_G1:
	case GP_PIXEL_G2:
	case GP_PIXEL_G4:
	case GP_PIXEL_G8:
		return GP_SavePGM(src, dst_path, callback);
	case GP_PIXEL_RGB888:
		return GP_SavePPM(src, dst_path, callback);
	default:
		if (GP_LineConvertible(src->pixel_type, ppm_save_pixels))
			return GP_SavePPM(src, dst_path, callback);

		errno = EINVAL;
		return 1;
	}
}
