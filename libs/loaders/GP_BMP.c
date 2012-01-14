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

#define BMP_HEADER_OFFSET 0x0e        /* info header offset */
#define BMP_PIXELS_OFFSET_OFFSET 0x0a /* offset to offset to pixel data */
#define BMP_PALETTE_OFFSET 54         /* offset to the palette */

#define BUF_TO_4(buf, off) \
	(buf[off] + (buf[off+1]<<8) + (buf[off+2]<<16) + (buf[off+3]<<24))

#define BUF_TO_2(buf, off) \
	(buf[off] + (buf[off+1]<<8))
	

struct bitmap_info_header {
	int32_t w;
	/* 
	 * If negative imge is top-down (bottom-up is default)
	 */
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

static uint32_t get_palette_size(struct bitmap_info_header *header)
{

	if (header->palette_colors)
		return header->palette_colors;

	return (1 << header->bpp);
}

static GP_RetCode read_bitmap_info_header(FILE *f,
                                          struct bitmap_info_header *header)
{
	if (fseek(f, BMP_HEADER_OFFSET, SEEK_SET)) {
		GP_DEBUG(1, "fseek(f, 0x%02x) failed: '%s'",
		            BMP_HEADER_OFFSET, strerror(errno));
		return GP_EBADFILE;
	}
	
	char buf[32];
	uint32_t header_size;

	/* Read info header size, header size determines header type */
	if (fread(buf, 1, 4, f) != 4) {
		GP_DEBUG(1, "Failed to read info header size");
		return GP_EBADFILE;
	}

	header_size = BUF_TO_4(buf, 0);

	GP_DEBUG(2, "BMP header type '%s'",
	            bitmap_info_header_size_name(header_size));

	switch (header_size) {
	case BITMAPCOREHEADER:
	case BITMAPCOREHEADER2:
		return GP_ENOIMPL;
	/* The bitmap core header only adds filelds to the end of the header */
	case BITMAPINFOHEADER:
	case BITMAPINFOHEADER2:
	case BITMAPINFOHEADER3:
	case BITMAPINFOHEADER4:
	break;
	default:
		GP_DEBUG(1, "Unknown header type, continuing anyway");
	break;
	};

	if (fread(buf, 1, sizeof(buf), f) != sizeof(buf)) {
		GP_DEBUG(1, "Failed to read bitmap info header");
		return 1;
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

/*
 * Reads palette, the format is R G B X, each one byte.
 */
GP_RetCode read_bitmap_palette(FILE *f, struct bitmap_info_header *header,
                               GP_Pixel *palette)
{
	uint32_t i;
	uint32_t palette_colors = get_palette_size(header);

	if (fseek(f, BMP_PALETTE_OFFSET, SEEK_SET)) {
		GP_DEBUG(1, "fseek(f, 0x%02x) failed: '%s'",
		            BMP_HEADER_OFFSET, strerror(errno));
		return GP_EBADFILE;
	}

	for (i = 0; i < palette_colors; i++) {
		uint8_t buf[4];

		if (fread(buf, 1, 4, f) != 4) {
			GP_DEBUG(1, "Failed to read palette %"PRIu32, i);
			return GP_EBADFILE;
		}
		
		palette[i] = GP_Pixel_CREATE_RGB888(buf[2], buf[1], buf[0]);
	
		GP_DEBUG(3, "Palette[%"PRIu32"] = [0x%02x, 0x%02x, 0x%02x]", i,
	        	    GP_Pixel_GET_R_RGB888(palette[i]),
	        	    GP_Pixel_GET_G_RGB888(palette[i]),
	        	    GP_Pixel_GET_B_RGB888(palette[i]));
	}

	return GP_ESUCCESS;
}

GP_RetCode seek_pixels_offset(FILE *f)
{
	uint32_t offset;

	if (fseek(f, BMP_PIXELS_OFFSET_OFFSET, SEEK_SET)) {
		GP_DEBUG(1, "fseek(f, 0x%02x) failed: '%s'",
		            BMP_HEADER_OFFSET, strerror(errno));
		return GP_EBADFILE;
	}

	uint8_t buf[4];

	if (fread(buf, 1, 4, f) != 4) {
		GP_DEBUG(1, "Failed to read pixel offset size");
		return GP_EBADFILE;
	}

	offset = BUF_TO_4(buf, 0);
	
	GP_DEBUG(2, "Offset to BMP pixels is 0x%x (%ubytes)",
	            offset, offset);

	if (fseek(f, offset, SEEK_SET)) {
		GP_DEBUG(1, "fseek(f, 0x%02x) failed: '%s'",
		            offset, strerror(errno));
		return GP_EBADFILE;
	}

	return GP_ESUCCESS;
}

GP_PixelType match_pixel_type(struct bitmap_info_header *header)
{
	switch (header->bpp) {
	case 1:
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

GP_RetCode read_palette(FILE *f, struct bitmap_info_header *header,
                        GP_Context *context, GP_ProgressCallback *callback)
{
	GP_RetCode ret;
	uint32_t palette_size = get_palette_size(header);
	GP_Pixel palette[get_palette_size(header)];

	if ((ret = read_bitmap_palette(f, header, palette)))
		return ret;
	
	if ((ret = seek_pixels_offset(f)))
		return ret;
	
	uint32_t row_size = bitmap_row_size(header);
	int32_t y;

	for (y = 0; y < GP_ABS(header->h); y++) {
		int32_t x;
		uint8_t row[row_size];

		if (fread(row, 1, row_size, f) != row_size) {
			GP_DEBUG(1, "Failed to read row %"PRId32, y);
			return GP_EBADFILE;
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
			return GP_EINTR;
		}
	}
	
	GP_ProgressCallbackDone(callback);
	return GP_ESUCCESS;
}

GP_RetCode read_rgb888(FILE *f, struct bitmap_info_header *header,
                       GP_Context *context, GP_ProgressCallback *callback)
{
	GP_RetCode ret;
	uint32_t row_size = 3 * header->w;
	int32_t y;
	
	if ((ret = seek_pixels_offset(f)))
		return ret;

	for (y = 0; y < GP_ABS(header->h); y++) {
		int32_t ry;

		if (header->h < 0)
			ry = y;
		else
			ry = GP_ABS(header->h) - 1 - y;
		
		uint8_t *row = GP_PIXEL_ADDR(context, 0, ry);

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

	if (header.compress_type != COMPRESS_RGB) {
		ret = GP_ENOIMPL;
		goto err;
	}

	if ((pixel_type = match_pixel_type(&header)) == GP_PIXEL_UNKNOWN) {
		ret = GP_ENOIMPL;
		goto err;
	}

	GP_Context *context;
	
	context = GP_ContextAlloc(header.w, GP_ABS(header.h), pixel_type);

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
