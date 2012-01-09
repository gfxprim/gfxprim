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
 * Copyright (C) 2009-2012 Cyril Hrubis <metan@ucw.cz>                       *
 *                                                                           *
 *****************************************************************************/

/*

  BMP loader.

  Thanks wikipedia for the format specification.

 */

#include <stdint.h>
#include <inttypes.h>

#include <ctype.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>

#include "core/GP_Debug.h"
#include "GP_BMP.h"

#define BMP_HEADER_OFFSET 0x0e        /* info header offset */
#define BMP_PIXELS_OFFSET_OFFSET 0x0a /* offset to offset to pixel data */

struct bitmap_info_header {
	int32_t w;
	int32_t h;
	uint16_t bpp;
	uint32_t compress_type;
	uint32_t palette_colors;
};

enum bitmap_compress {
	COMPRESS_NONE           = 0,
	COMPRESS_RLE8           = 1,
	COMPRESS_RLE4           = 2,
	COMPRESS_BITFIELDS      = 3,
	COMPRESS_JPEG           = 4, /* only for printers */
	COMPRESS_PNG            = 5, /* only for printers */
	COMPRESS_ALPHABITFIELDS = 6,
	COMPRESS_MAX = COMPRESS_ALPHABITFIELDS,
};

static const char *bitmap_compress_names[] = {
	"None",
	"RLE8",
	"RLE4",
	"BITFIELDS",
	"JPEG",
	"PNG",
	"ALPHABITFIELDS",
};

static const char *bitmap_compress_name(uint32_t compress)
{
	if (compress >= COMPRESS_MAX)
		return "Unknown";

	return bitmap_compress_names[compress];
}

enum bitmap_info_header_sizes {
	BITMAPCOREHEADER  = 12,  /* old OS/2 format + win 3.0             */
	BITMAPCOREHEADER2 = 64,  /* OS/2                                  */
	BITMAPINFOHEADER  = 40,  /* most common                           */
	BITMAPINFOHEADER2 = 52,  /* Undocummented                         */
	BITMAPINFOHEADER3 = 56,  /* Undocummented                         */
	BITMAPINFOHEADER4 = 108, /* adds color space + gamma - win 95/NT4 */
	BITMAPINFOHEADER5 = 124, /* adds ICC color profiles win 98+       */
};

static const char *bitmap_info_header_size_name(uint32_t size)
{
	switch (size) {
	case BITMAPCOREHEADER:
		return "BitmapCoreHeader";
	case BITMAPCOREHEADER2:
		return "BitmapCoreHeader2";
	case BITMAPINFOHEADER:
		return "BitmapInfoHeader";
	case BITMAPINFOHEADER2:
		return "BitmapInfoHeader2";
	case BITMAPINFOHEADER3:
		return "BitmapInfoHeader3";
	case BITMAPINFOHEADER4:
		return "BitmapInfoHeader4";
	case BITMAPINFOHEADER5:
		return "BitmapInfoHeader5";
	}

	return "Unknown";
}

static GP_RetCode read_bitmap_info_header(FILE *f,
                                          struct bitmap_info_header *header)
{
	if (fseek(f, BMP_HEADER_OFFSET, SEEK_SET)) {
		GP_DEBUG(1, "fseek(f, 0x%02x) failed: '%s'",
		            BMP_HEADER_OFFSET, strerror(errno));
		return GP_EBADFILE;
	}
	
	char buf[36];
	uint32_t header_size;

	/* Read info header size, header size determines header type */
	if (fread(buf, 1, 4, f) != 4) {
		GP_DEBUG(1, "Failed to read info header size");
		return GP_EBADFILE;
	}

	header_size = buf[0] + (buf[1]<<8) + (buf[2]<<16) + (buf[3]<<24);

	GP_DEBUG(2, "BMP header type '%s'",
	            bitmap_info_header_size_name(header_size));

	switch (header_size) {
	case BITMAPCOREHEADER:
	case BITMAPCOREHEADER2:
		return GP_ENOIMPL;
	/* The bitmap core header only adds filelds to the end of the header */
	default:
	break;
	};

	if (fread(buf, 1, sizeof(buf), f) != sizeof(buf)) {
		GP_DEBUG(1, "Failed to read bitmap info header");
		return 1;
	}

	header->w = buf[0] + (buf[1]<<8) + (buf[2]<<16) + (buf[3]<<24);
	header->h = buf[4] + (buf[5]<<8) + (buf[6]<<16) + (buf[7]<<24);
	
	header->bpp = buf[10] + (buf[11]<<8);
	
	header->compress_type = buf[12] + (buf[13]<<8) +
	                       (buf[14]<<16) + (buf[15]<<24);
	
	header->palette_colors = buf[32] + (buf[33]<<8) +
	                        (buf[34]<<16) + (buf[35]<<24);

	GP_DEBUG(2, "Have BMP bitmap size %"PRId32"x%"PRId32" %"PRIu16" "
	            "bpp, %"PRIu32" pallete colors, '%s' compression",
		    header->w, header->h, header->bpp, header->palette_colors,
		    bitmap_compress_name(header->compress_type));

	return 0;
}

GP_RetCode read_pixels_offset(FILE *f, uint32_t *offset)
{
	if (fseek(f, BMP_PIXELS_OFFSET_OFFSET, SEEK_SET)) {
		GP_DEBUG(1, "fseek(f, 0x%02x) failed: '%s'",
		            BMP_HEADER_OFFSET, strerror(errno));
		return GP_EBADFILE;
	}

	char buf[4];

	/* Read info header size, header size determines header type */
	if (fread(buf, 1, 4, f) != 4) {
		GP_DEBUG(1, "Failed to read info header size");
		return GP_EBADFILE;
	}

	*offset = buf[0] + (buf[1]<<8) + (buf[2]<<16) + (buf[3]<<24);

	return GP_ESUCCESS;
}

GP_PixelType match_pixel_type(struct bitmap_info_header *header)
{
	switch (header->bpp) {
	case 1:
		/* context->bitendian = 1 */
		return GP_PIXEL_G1;
	case 24:
		return GP_PIXEL_RGB888;
	}

	return GP_PIXEL_UNKNOWN;
}

GP_RetCode read_g1(FILE *f, struct bitmap_info_header *header,
                   GP_Context *context, GP_ProgressCallback *callback)
{
	int32_t y;
	uint32_t row_size = header->w / 8 + !!(header->w%8);

	context->bit_endian = 1;

	for (y = header->h - 1; y >= 0; y--) {
		uint8_t *row = GP_PIXEL_ADDR(context, 0, y);

		if (fread(row, 1, row_size, f) != row_size)
			return GP_EBADFILE;
	
		/* Rows are four byte aligned */
		switch (row_size % 4) {
		case 1:
			fgetc(f);
		case 2:
			fgetc(f);
		case 3:
			fgetc(f);
		case 0:
		break;
		}
		
		if (GP_ProgressCallbackReport(callback, header->h - y -1,
		                              context->h, context->w)) {
			GP_DEBUG(1, "Operation aborted");
			return GP_EINTR;
		}
	}
	
	GP_ProgressCallbackDone(callback);
	return GP_ESUCCESS;
}

GP_RetCode read_rgb888(FILE *f, struct bitmap_info_header *header,
                       GP_Context *context, GP_ProgressCallback *callback)
{
	int32_t y;
	uint32_t row_size = 3 * header->w;

	for (y = header->h - 1; y >= 0; y--) {
		uint8_t *row = GP_PIXEL_ADDR(context, 0, y);

		if (fread(row, 1, row_size, f) != row_size)
			return GP_EBADFILE;
	
		/* Rows are four byte aligned */
		switch (row_size % 4) {
		case 1:
			fgetc(f);
		case 2:
			fgetc(f);
		case 3:
			fgetc(f);
		case 0:
		break;
		}
		
		if (GP_ProgressCallbackReport(callback, header->h - y -1,
		                              context->h, context->w)) {
			GP_DEBUG(1, "Operation aborted");
			return GP_EINTR;
		}
	}

	GP_ProgressCallbackDone(callback);
	return GP_ESUCCESS;
}

GP_RetCode read_bitmap_pixels(FILE *f, struct bitmap_info_header *header,
                              GP_Context *context, GP_ProgressCallback *callback)
{
	uint32_t pixels_offset;
	GP_RetCode ret;

	if ((ret = read_pixels_offset(f, &pixels_offset)))
		return ret;

	GP_DEBUG(2, "Offset to BMP pixels is 0x%x (%ubytes)",
	            pixels_offset, pixels_offset);

	if (fseek(f, pixels_offset, SEEK_SET)) {
		GP_DEBUG(1, "fseek(f, 0x%02x (pixels_offset)) failed: '%s'",
		            pixels_offset, strerror(errno));
		return GP_EBADFILE;
	}

	switch (header->bpp) {
	case 1:
		return read_g1(f, header, context, callback);
	case 24:
		return read_rgb888(f, header, context, callback);
	}

	return GP_ENOIMPL;
}

GP_RetCode GP_OpenBMP(const char *src_path, FILE **f,
                      GP_Size *w, GP_Size *h, GP_PixelType *pixel_type)
{
	GP_RetCode ret = GP_EBADFILE;
	
	*f = fopen(src_path, "rb");

	if (*f == NULL) {
		GP_DEBUG(1, "Failed to open '%s' : %s",
		            src_path, strerror(errno));
		return GP_EBADFILE;
	}

	int ch1 = fgetc(*f);
	int ch2 = fgetc(*f);

	if (ch1 != 'B' || ch2 != 'M') {
		GP_DEBUG(1, "Unexpected bitmap header 0x%02x (%c) 0x%02x (%c)",
		            ch1, isascii(ch1) ? ch1 : ' ',
			    ch2, isascii(ch2) ? ch2 : ' ');
		goto err;
	}
	
	if (w != NULL || h != NULL || pixel_type != NULL) {
		struct bitmap_info_header header;
	
		if ((ret = read_bitmap_info_header(*f, &header)))
			goto err;

		if (w != NULL)
			*w = header.w;
	
		if (h != NULL)
			*h = header.h;
	}

	return GP_ESUCCESS;
err:
	fclose(*f);
	return GP_EBADFILE;
}

GP_RetCode GP_ReadBMP(FILE *f, GP_Context **res,
                      GP_ProgressCallback *callback)
{
	struct bitmap_info_header header;
	GP_RetCode ret;
	GP_PixelType pixel_type;

	if ((ret = read_bitmap_info_header(f, &header)))
		goto err;

	if (header.compress_type != COMPRESS_NONE) {
		ret = GP_ENOIMPL;
		goto err;
	}

	if ((pixel_type = match_pixel_type(&header)) == GP_PIXEL_UNKNOWN) {
		ret = GP_ENOIMPL;
		goto err;
	}

	GP_Context *context = GP_ContextAlloc(header.w, header.h, pixel_type);

	if (context == NULL) {
		ret = GP_ENOMEM;
		goto err;
	}

	if ((ret = read_bitmap_pixels(f, &header, context, callback)))
		goto err1;

	*res = context;

	return GP_ESUCCESS;
err1:
	GP_ContextFree(context);
err:
	fclose(f);
	return ret;
}

GP_RetCode GP_LoadBMP(const char *src_path, GP_Context **res,
                      GP_ProgressCallback *callback)
{
	FILE *f;
	GP_RetCode ret;

	if ((ret = GP_OpenBMP(src_path, &f, NULL, NULL, NULL)))
		return ret;

	return GP_ReadBMP(f, res, callback);
}
