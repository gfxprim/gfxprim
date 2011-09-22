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
 * Copyright (C) 2009-2010 Cyril Hrubis <metan@ucw.cz>                       *
 *                                                                           *
 *****************************************************************************/

/*

  PNG image support using libpng.
  
 */

#include <stdint.h>
#include <inttypes.h>

#include <errno.h>
#include <string.h>

#include <png.h>

#include <GP_Context.h>
#include <GP_Debug.h>

GP_RetCode GP_OpenPNG(const char *src_path, FILE **f)
{
	uint8_t sig[8];

	*f = fopen(src_path, "r");

	if (*f == NULL) {
		GP_DEBUG(1, "Failed to open '%s' : %s",
		            src_path, strerror(errno));
		return GP_EBADFILE;
	}

	if (fread(sig, 1, 8, *f) <= 0) {
		GP_DEBUG(1, "Failed to read '%s' : %s",
		            src_path, strerror(errno));
		goto err;
	}

	if (png_sig_cmp(sig, 0, 8)) {
		GP_DEBUG(1, "Invalid file header, '%s' not a PNG image?",
		            src_path);
		goto err;	
	}

	GP_DEBUG(1, "Found PNG signature in '%s'", src_path);

	return GP_ESUCCESS;
err:
	fclose(*f);
	*f = NULL;
	return GP_EBADFILE;
}

GP_RetCode GP_ReadPNG(FILE *f, GP_Context **res)
{
	png_structp png;
	png_infop   png_info = NULL;
	png_uint_32 w, h;
	int depth, color_type;
	GP_PixelType pixel_type = GP_PIXEL_UNKNOWN;
	GP_RetCode ret;

	png = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);

	if (png == NULL) {
		GP_DEBUG(1, "Failed to allocate PNG read buffer");
		ret = GP_ENOMEM;
		goto err1;
	}

	png_info = png_create_info_struct(png);

	if (png_info == NULL) {
		GP_DEBUG(1, "Failed to allocate PNG info buffer");
		ret = GP_ENOMEM;
		goto err2;
	}

	if (setjmp(png_jmpbuf(png))) {
		GP_DEBUG(1, "Failed to read PNG file :(");
		ret = GP_EBADFILE;
		goto err2;
	}

	png_init_io(png, f);
	png_set_sig_bytes(png, 8);
	png_read_info(png, png_info);

	png_get_IHDR(png, png_info, &w, &h, &depth,
	             &color_type, NULL, NULL, NULL);

	GP_DEBUG(2, "Have %s%s PNG%s size %ux%u depth %i",
	         color_type & PNG_COLOR_MASK_PALETTE ? "pallete " : "",
	         color_type & PNG_COLOR_MASK_COLOR ? "color" : "gray",
		 color_type & PNG_COLOR_MASK_ALPHA ? " with alpha channel" : "",
		 w, h, depth);

	switch (color_type) {
	case PNG_COLOR_TYPE_GRAY:
		switch (depth) {
		case 1:
			pixel_type = GP_PIXEL_G1;
		break;
		case 2:
			pixel_type = GP_PIXEL_G2;
		break;
		case 4:
			pixel_type = GP_PIXEL_G4;
		break;
		case 8:
			pixel_type = GP_PIXEL_G8;
		break;
		}
	break;
	case PNG_COLOR_TYPE_RGB:
		
		png_set_bgr(png);
		
		switch (depth) {
		case 8:
			pixel_type = GP_PIXEL_RGB888;
		break;
		}
	break;
	case PNG_COLOR_TYPE_PALETTE:
		/* Grayscale with BPP < 8 is usually saved as palette */
		if (png_get_channels(png, png_info) == 1) {
			switch (depth) {
			case 1:
				png_set_packswap(png);
				pixel_type = GP_PIXEL_G1;
			break;
			}
		}
	break;
	}

	if (pixel_type == GP_PIXEL_UNKNOWN) {
		ret = GP_ENOIMPL;
		goto err2;
	}

	*res = GP_ContextAlloc(w, h, pixel_type);

	if (*res == NULL) {
		ret = GP_ENOMEM;
		goto err2;
	}

	uint32_t y;

	for (y = 0; y < h; y++) {
		png_bytep addr = GP_PIXEL_ADDR(*res, 0, y);
		png_read_rows(png, &addr, NULL, 1);
	}

	ret = GP_ESUCCESS;
err2:
	png_destroy_read_struct(&png, png_info ? &png_info : NULL, NULL);
err1:
	fclose(f);
	return ret;
}

GP_RetCode GP_LoadPNG(const char *src_path, GP_Context **res)
{
	FILE *f;
	GP_RetCode ret;

	if ((ret = GP_OpenPNG(src_path, &f)))
		return ret;

	return GP_ReadPNG(f, res);
}
