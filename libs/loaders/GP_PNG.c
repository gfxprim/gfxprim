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
 * Copyright (C) 2009-2011 Cyril Hrubis <metan@ucw.cz>                       *
 *                                                                           *
 *****************************************************************************/

/*

  PNG image support using libpng.
  
 */

#include <stdint.h>
#include <inttypes.h>

#include <errno.h>
#include <string.h>

#include "../../config.h"
#include "core/GP_Debug.h"

#include "GP_PNG.h"

#ifdef HAVE_LIBPNG

#include <png.h>

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

static const char *interlace_type_name(int interlace)
{
	switch (interlace) {
	case PNG_INTERLACE_NONE:
		return "none";
	case PNG_INTERLACE_ADAM7:
		return "adam7";
	default:
		return "unknown";
	}
}

GP_RetCode GP_ReadPNG(FILE *f, GP_Context **res,
                      GP_ProgressCallback *callback)
{
	png_structp png;
	png_infop   png_info = NULL;
	png_uint_32 w, h;
	int depth, color_type, interlace_type;
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
	             &color_type, &interlace_type, NULL, NULL);

	GP_DEBUG(2, "Have %s%s interlace %s PNG%s size %ux%u depth %i",
	         interlace_type_name(interlace_type),
	         color_type & PNG_COLOR_MASK_PALETTE ? " pallete " : "",
	         color_type & PNG_COLOR_MASK_COLOR ? "color" : "gray",
		 color_type & PNG_COLOR_MASK_ALPHA ? " with alpha channel" : "",
		 (unsigned int)w, (unsigned int)h, depth);

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
		
		/* Convert everything else to RGB888 */
		//TODO: add palette matching to G2 G4 and G8
		png_set_palette_to_rgb(png);
		png_set_bgr(png);
		pixel_type = GP_PIXEL_RGB888;
	break;
	}

	if (pixel_type == GP_PIXEL_UNKNOWN) {
		GP_DEBUG(1, "Unimplemented png format");
		ret = GP_ENOIMPL;
		goto err2;
	}

	*res = GP_ContextAlloc(w, h, pixel_type);

	if (*res == NULL) {
		ret = GP_ENOMEM;
		goto err2;
	}

	uint32_t y;
	
	/* start the actuall reading */
	for (y = 0; y < h; y++) {
		png_bytep row = GP_PIXEL_ADDR(*res, 0, y);
		png_read_rows(png, &row, NULL, 1);

		if (GP_ProgressCallbackReport(callback, y, h, w)) {
			GP_DEBUG(1, "Operation aborted");
			png_destroy_read_struct(&png, &png_info, NULL);
			fclose(f);
			GP_ContextFree(*res);
			return GP_EINTR;
		}
			
	}

	GP_ProgressCallbackDone(callback);

	ret = GP_ESUCCESS;
err2:
	png_destroy_read_struct(&png, png_info ? &png_info : NULL, NULL);
err1:
	fclose(f);
	return ret;
}

GP_RetCode GP_LoadPNG(const char *src_path, GP_Context **res,
                      GP_ProgressCallback *callback)
{
	FILE *f;
	GP_RetCode ret;

	if ((ret = GP_OpenPNG(src_path, &f)))
		return ret;

	return GP_ReadPNG(f, res, callback);
}

GP_RetCode GP_SavePNG(const char *dst_path, const GP_Context *src,
                      GP_ProgressCallback *callback)
{
	FILE *f;
	GP_RetCode ret;
	png_structp png;
	png_infop png_info = NULL;

	if (src->pixel_type != GP_PIXEL_RGB888) {
		GP_DEBUG(1, "Can't save png with pixel type %s",
		         GP_PixelTypeName(src->pixel_type));
		return GP_ENOIMPL;
	}

	f = fopen(dst_path, "wb");

	if (f == NULL) {
		GP_DEBUG(1, "Failed to open '%s' for writing: %s",
		         dst_path, strerror(errno));
		return GP_EBADFILE;
	}

	png = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);

	if (png == NULL) {
		GP_DEBUG(1, "Failed to allocate PNG write buffer");
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
		GP_DEBUG(1, "Failed to write PNG file :(");
		ret = GP_EBADFILE;
		goto err2;
	}

	png_init_io(png, f);
	png_set_IHDR(png, png_info, src->w, src->h, 8, PNG_COLOR_TYPE_RGB,
	             PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT,
		     PNG_FILTER_TYPE_DEFAULT);

	/* start the actuall writing */
	png_write_info(png, png_info);
	png_set_bgr(png);

	uint32_t y;

	for (y = 0; y < src->h; y++) {
		png_bytep row = GP_PIXEL_ADDR(src, 0, y);
		png_write_row(png, row);

		if (GP_ProgressCallbackReport(callback, y, src->h, src->w)) {
			GP_DEBUG(1, "Operation aborted");
			ret = GP_EINTR;
			goto err2;
		}
			
	}

	png_write_end(png, png_info);
	png_destroy_write_struct(&png, &png_info);

	if (fclose(f)) {
		GP_DEBUG(1, "Failed to close file '%s': %s",
		         dst_path, strerror(errno));
		return GP_EBADFILE;
	}

	GP_ProgressCallbackDone(callback);
	return GP_ESUCCESS;
err2:
	png_destroy_write_struct(&png, png_info == NULL ? NULL : &png_info);
err1:
	fclose(f);
	unlink(dst_path);
	return ret;
}

#else

GP_RetCode GP_OpenPNG(const char GP_UNUSED(*src_path),
                      FILE GP_UNUSED(**f))
{
	return GP_ENOIMPL;
}

GP_RetCode GP_ReadPNG(FILE GP_UNUSED(*f),
                      GP_Context GP_UNUSED(**res),
                      GP_ProgressCallback GP_UNUSED(*callback))
{
	return GP_ENOIMPL;
}

GP_RetCode GP_LoadPNG(const char GP_UNUSED(*src_path),
                      GP_Context GP_UNUSED(**res),
                      GP_ProgressCallback GP_UNUSED(*callback))
{
	return GP_ENOIMPL;
}

GP_RetCode GP_SavePNG(const char GP_UNUSED(*dst_path),
                      const GP_Context GP_UNUSED(*src),
                      GP_ProgressCallback GP_UNUSED(*callback))
{
	return GP_ENOIMPL;
}

#endif /* HAVE_LIBPNG */
