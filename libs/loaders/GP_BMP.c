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
#include "core/GP_Pixel.h"
#include "core/GP_GetPutPixel.h"
#include "GP_BMP.h"

#define BMP_HEADER_OFFSET  0x0a       /* info header offset - 4 bytes */

#define BUF_TO_4(buf, off) \
	(buf[off] + (buf[off+1]<<8) + (buf[off+2]<<16) + (buf[off+3]<<24))

#define BUF_TO_2(buf, off) \
	(buf[off] + (buf[off+1]<<8))
	

struct bitmap_info_header {
	/*
	 * Offset to image data.
	 */
	uint32_t pixel_offset;

	/*
	 * Header size (palette is on offset header_size + 14)
	 */
	uint32_t header_size;

	/* 
	 * Image size in pixels.
	 * If h is negative image is top-down (bottom-up is default)
	 */
	int32_t w;
	int32_t h;               
	
	uint16_t bpp;
	uint32_t compress_type;
	/* 
	 * if 0 image uses whole range (2^bpp colors)
	 */
	uint32_t palette_colors; 
};

enum bitmap_compress {
	COMPRESS_RGB            = 0, /* uncompressed              */
	COMPRESS_RLE8           = 1, /* run-length encoded bitmap */
	COMPRESS_RLE4           = 2, /* run-length encoded bitmap */
	COMPRESS_BITFIELDS      = 3, /* bitfield for each channel */
	COMPRESS_JPEG           = 4, /* only for printers */
	COMPRESS_PNG            = 5, /* only for printers */
	COMPRESS_ALPHABITFIELDS = 6,
	COMPRESS_MAX = COMPRESS_ALPHABITFIELDS,
};

static const char *bitmap_compress_names[] = {
	"RGB",
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

static const char *bitmap_header_size_name(uint32_t size)
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

static uint32_t get_palette_size(struct bitmap_info_header *header)
{

	if (header->palette_colors)
		return header->palette_colors;

	return (1 << header->bpp);
}

static int read_bitmap_info_header(FILE *f, struct bitmap_info_header *header)
{
	uint8_t buf[36];

	if (fread(buf, 1, sizeof(buf), f) != sizeof(buf)) {
		GP_DEBUG(1, "Failed to read bitmap info header");
		return EIO;
	}

	header->w              = BUF_TO_4(buf, 0);
	header->h              = BUF_TO_4(buf, 4);
	header->bpp            = BUF_TO_2(buf, 10);
	header->compress_type  = BUF_TO_4(buf, 12);
	header->palette_colors = BUF_TO_4(buf, 28);

	uint16_t nr_planes = BUF_TO_2(buf, 8);

	/* This must be 1 according to specs */
	if (nr_planes != 1)
		GP_DEBUG(1, "Number of planes is %"PRId16" should be 1",
		            nr_planes);

	GP_DEBUG(2, "Have BMP bitmap size %"PRId32"x%"PRId32" %"PRIu16" "
	            "bpp, %"PRIu32" pallete colors, '%s' compression",
		    header->w, header->h, header->bpp,
	            get_palette_size(header),
	            bitmap_compress_name(header->compress_type));

	return 0;
}

static int read_bitmap_core_header(FILE *f, struct bitmap_info_header *header)
{
	uint8_t buf[12];

	if (fread(buf, 1, sizeof(buf), f) != sizeof(buf)) {
		GP_DEBUG(1, "Failed to read bitmap core header");
		return EIO;
	}
	
	header->w = BUF_TO_2(buf, 0);
	header->h = BUF_TO_2(buf, 2);
	header->bpp = BUF_TO_2(buf, 6);
	header->compress_type = COMPRESS_RGB;
	header->palette_colors = 0;

	uint16_t nr_planes = BUF_TO_2(buf, 4);

	/* This must be 1 according to specs */
	if (nr_planes != 1)
		GP_DEBUG(1, "Number of planes is %"PRId16" should be 1",
		            nr_planes);

	GP_DEBUG(2, "Have BMP bitmap size %"PRId32"x%"PRId32" %"PRIu16" bpp",
	            header->h, header->w, header->bpp);

	return 0;
}

static int read_bitmap_header(FILE *f, struct bitmap_info_header *header)
{
	uint8_t buf[8];
	int err;

	if (fseek(f, BMP_HEADER_OFFSET, SEEK_SET)) {
		err = errno;
		GP_DEBUG(1, "fseek(f, 0x%02x) failed: '%s'",
		            BMP_HEADER_OFFSET, strerror(errno));
		return err;
	}

	/* Read info header size, header size determines header type */
	if (fread(buf, 1, sizeof(buf), f) != sizeof(buf)) {
		GP_DEBUG(1, "Failed to read info header size");
		return EIO;
	}

	header->pixel_offset = BUF_TO_4(buf, 0);
	header->header_size = BUF_TO_4(buf, 4);

	GP_DEBUG(2, "BMP header type '%s'",
	            bitmap_header_size_name(header->header_size));

	switch (header->header_size) {
	case BITMAPCOREHEADER:
		err = read_bitmap_core_header(f, header);
	break;
	case BITMAPCOREHEADER2:
		return ENOSYS;
	/* The bitmap core header only adds filelds to the end of the header */
	case BITMAPINFOHEADER:
	case BITMAPINFOHEADER2:
	case BITMAPINFOHEADER3:
	case BITMAPINFOHEADER4:
		err = read_bitmap_info_header(f, header);
	break;
	default:
		GP_DEBUG(1, "Unknown header type, continuing anyway");
		err = read_bitmap_info_header(f, header);
	break;
	};
	
	return err;
}

/*
 * Reads palette, the format is R G B X, each one byte.
 */
static int read_bitmap_palette(FILE *f, struct bitmap_info_header *header,
                               GP_Pixel *palette)
{
	uint32_t palette_colors = get_palette_size(header);
	uint32_t palette_offset = header->header_size + 14;
	uint8_t pixel_size;
	uint32_t i;
	int err;

	switch (header->header_size) {
	case BITMAPCOREHEADER:
		pixel_size = 3;
	break;
	default:
		pixel_size = 4;
	break;
	}

	GP_DEBUG(2, "Offset to BMP palette is 0x%x (%ubytes) "
	            "pixel size %"PRIu8"bytes",
		    palette_offset, palette_offset, pixel_size);

	if (fseek(f, palette_offset, SEEK_SET)) {
		err = errno;
		GP_DEBUG(1, "fseek(f, 0x%02x) failed: '%s'",
		            BMP_HEADER_OFFSET, strerror(errno));
		return err;
	}

	for (i = 0; i < palette_colors; i++) {
		uint8_t buf[4];

		if (fread(buf, 1, pixel_size, f) != pixel_size) {
			GP_DEBUG(1, "Failed to read palette %"PRIu32, i);
			return EIO;
		}
		
		palette[i] = GP_Pixel_CREATE_RGB888(buf[2], buf[1], buf[0]);
	
		GP_DEBUG(3, "Palette[%"PRIu32"] = [0x%02x, 0x%02x, 0x%02x]", i,
	        	    GP_Pixel_GET_R_RGB888(palette[i]),
	        	    GP_Pixel_GET_G_RGB888(palette[i]),
	        	    GP_Pixel_GET_B_RGB888(palette[i]));
	}

	return 0;
}

static int seek_pixels_offset(struct bitmap_info_header *header, FILE *f)
{
	int err;

	GP_DEBUG(2, "Offset to BMP pixels is 0x%x (%ubytes)",
	            header->pixel_offset, header->pixel_offset);

	if (fseek(f, header->pixel_offset, SEEK_SET)) {
		err = errno;
		GP_DEBUG(1, "fseek(f, 0x%02x) failed: '%s'",
		            header->pixel_offset, strerror(errno));
		return err;
	}

	return 0;
}

static GP_PixelType match_pixel_type(struct bitmap_info_header *header)
{
	switch (header->bpp) {
	case 1:
	case 2:
	case 4:
	case 8:
	case 24:
		return GP_PIXEL_RGB888;
	}

	return GP_PIXEL_UNKNOWN;
}

/*
 * Returns four byte aligned row size for palette formats.
 */
static uint32_t bitmap_row_size(struct bitmap_info_header *header)
{
	uint32_t row_size = 0;
	
	/* align width to whole bytes */
	switch (header->bpp) {
	case 1:
		row_size = header->w / 8 + !!(header->w%8);
	break;
	case 2:
		row_size = header->w / 4 + !!(header->w%4);
	break;
	case 4:
		row_size = header->w / 2 + !!(header->w%2);
	break;
	case 8:
		row_size = header->w;
	break;
	}

	/* align row_size to four byte boundary */
	switch (row_size % 4) {
	case 1:
		row_size++;
	case 2:
		row_size++;
	case 3:
		row_size++;
	case 0:
	break;
	}

	GP_DEBUG(2, "bpp = %"PRIu16", width = %"PRId32", row_size = %"PRIu32,
	            header->bpp, header->w, row_size);
	
	return row_size;
}

static uint8_t get_idx(struct bitmap_info_header *header,
                       uint8_t row[], int32_t x)
{
	switch (header->bpp) {
	case 1:
		return !!(row[x/8] & (1<<(7 - x%8))); 
	case 2:
		return (row[x/4] >> (2*(3 - x%4))) & 0x03;
	case 4:
		return (row[x/2] >> (4*(!(x%2)))) & 0x0f;
	break;
	case 8:
		return row[x];
	}

	return 0;
}

static int read_palette(FILE *f, struct bitmap_info_header *header,
                        GP_Context *context, GP_ProgressCallback *callback)
{
	uint32_t palette_size = get_palette_size(header);
	GP_Pixel palette[get_palette_size(header)];
	int err;

	if ((err = read_bitmap_palette(f, header, palette)))
		return err;
	
	if ((err = seek_pixels_offset(header, f)))
		return err;
	
	uint32_t row_size = bitmap_row_size(header);
	int32_t y;

	for (y = 0; y < GP_ABS(header->h); y++) {
		int32_t x;
		uint8_t row[row_size];

		if (fread(row, 1, row_size, f) != row_size) {
			GP_DEBUG(1, "Failed to read row %"PRId32, y);
			return EIO;
		}
	
		for (x = 0; x < header->w; x++) {
			uint8_t idx = get_idx(header, row, x);
			GP_Pixel p;

			if (idx >= palette_size) {
				GP_DEBUG(1, "Index out of palette, ignoring");
				p = 0;
			} else {
				p = palette[idx];
			}
			
			int32_t ry;

			if (header->h < 0)
				ry = y;
			else
				ry = GP_ABS(header->h) - 1 - y;

			GP_PutPixel_Raw_24BPP(context, x, ry, p);
		}
		
		if (GP_ProgressCallbackReport(callback, header->h - y -1,
		                              context->h, context->w)) {
			GP_DEBUG(1, "Operation aborted");
			return ECANCELED;
		}
	}
	
	GP_ProgressCallbackDone(callback);
	return 0;
}

static int read_rgb888(FILE *f, struct bitmap_info_header *header,
                       GP_Context *context, GP_ProgressCallback *callback)
{
	uint32_t row_size = 3 * header->w;
	int32_t y;
	int err;
	
	if ((err = seek_pixels_offset(header, f)))
		return err;

	for (y = 0; y < GP_ABS(header->h); y++) {
		int32_t ry;

		if (header->h < 0)
			ry = y;
		else
			ry = GP_ABS(header->h) - 1 - y;
		
		uint8_t *row = GP_PIXEL_ADDR(context, 0, ry);

		if (fread(row, 1, row_size, f) != row_size) {
			GP_DEBUG(1, "Failed to read row %"PRId32, y);
			return EIO;
		}

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
			return ECANCELED;
		}
	}

	GP_ProgressCallbackDone(callback);
	return 0;
}

static int read_bitmap_pixels(FILE *f, struct bitmap_info_header *header,
                              GP_Context *context, GP_ProgressCallback *callback)
{
	switch (header->bpp) {
	case 1:
	/* I haven't been able to locate 2bpp palette bmp file => not tested */
	case 2:
	case 4:
	case 8:
		return read_palette(f, header, context, callback);
	case 24:
		return read_rgb888(f, header, context, callback);
	}

	return ENOSYS;
}

int GP_MatchBMP(const void *buf)
{
	return !memcmp(buf, "BM", 2);
}

int GP_OpenBMP(const char *src_path, FILE **f,
               GP_Size *w, GP_Size *h, GP_PixelType *pixel_type)
{
	int err;

	*f = fopen(src_path, "rb");

	if (*f == NULL) {
		err = errno;
		GP_DEBUG(1, "Failed to open '%s' : %s",
		            src_path, strerror(errno));
		goto err2;
	}

	int ch1 = fgetc(*f);
	int ch2 = fgetc(*f);

	if (ch1 != 'B' || ch2 != 'M') {
		GP_DEBUG(1, "Unexpected bitmap header 0x%02x (%c) 0x%02x (%c)",
		            ch1, isascii(ch1) ? ch1 : ' ',
			    ch2, isascii(ch2) ? ch2 : ' ');
		err = EIO;
		goto err1;
	}
	
	if (w != NULL || h != NULL || pixel_type != NULL) {
		struct bitmap_info_header header;
	
		if ((err = read_bitmap_header(*f, &header)))
			goto err1;

		if (w != NULL)
			*w = header.w;
	
		if (h != NULL)
			*h = header.h;
	
		if (pixel_type != NULL)
			*pixel_type = match_pixel_type(&header); 
	}

	return 0;
err1:
	fclose(*f);
err2:
	errno = err;
	return 1;
}

GP_Context *GP_ReadBMP(FILE *f, GP_ProgressCallback *callback)
{
	struct bitmap_info_header header;
	GP_PixelType pixel_type;
	GP_Context *context;
	int err;

	if ((err = read_bitmap_header(f, &header)))
		goto err1;

	if (header.compress_type != COMPRESS_RGB) {
		GP_DEBUG(2, "Unknown compression type");
		err = ENOSYS;
		goto err1;
	}

	if ((pixel_type = match_pixel_type(&header)) == GP_PIXEL_UNKNOWN) {
		GP_DEBUG(2, "Unknown pixel type");
		err = ENOSYS;
		goto err1;
	}
	
	context = GP_ContextAlloc(header.w, GP_ABS(header.h), pixel_type);

	if (context == NULL) {
		err = ENOMEM;
		goto err1;
	}

	if ((err = read_bitmap_pixels(f, &header, context, callback)))
		goto err2;

	return context;
err2:
	GP_ContextFree(context);
err1:
	fclose(f);
	errno = err;
	return NULL;
}

GP_Context *GP_LoadBMP(const char *src_path, GP_ProgressCallback *callback)
{
	FILE *f;

	if (GP_OpenBMP(src_path, &f, NULL, NULL, NULL))
		return NULL;

	return GP_ReadBMP(f, callback);
}
