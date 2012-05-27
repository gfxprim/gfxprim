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

#include "core/GP_BitSwap.h"

int GP_OpenPNG(const char *src_path, FILE **f)
{
	uint8_t sig[8];
	int err;

	*f = fopen(src_path, "r");

	if (*f == NULL) {
		err = errno;
		GP_DEBUG(1, "Failed to open '%s' : %s",
		            src_path, strerror(errno));
		goto err1;
	}

	if (fread(sig, 1, 8, *f) <= 0) {
		err = errno;
		GP_DEBUG(1, "Failed to read '%s' : %s",
		            src_path, strerror(errno));
		goto err2;
	}

	if (png_sig_cmp(sig, 0, 8)) {
		GP_DEBUG(1, "Invalid file header, '%s' not a PNG image?",
		            src_path);
		err = EILSEQ;
		goto err2;	
	}

	GP_DEBUG(1, "Found PNG signature in '%s'", src_path);

	return 0;
err2:
	fclose(*f);
err1:
	errno = err;
	return 1;
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

GP_Context *GP_ReadPNG(FILE *f, GP_ProgressCallback *callback)
{
	png_structp png;
	png_infop png_info = NULL;
	png_uint_32 w, h;
	int depth, color_type, interlace_type;
	GP_PixelType pixel_type = GP_PIXEL_UNKNOWN;
	GP_Context *res;
	int err;

	png = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);

	if (png == NULL) {
		GP_DEBUG(1, "Failed to allocate PNG read buffer");
		err = ENOMEM;
		goto err1;
	}

	png_info = png_create_info_struct(png);

	if (png_info == NULL) {
		GP_DEBUG(1, "Failed to allocate PNG info buffer");
		err = ENOMEM;
		goto err2;
	}

	if (setjmp(png_jmpbuf(png))) {
		GP_DEBUG(1, "Failed to read PNG file :(");
		//TODO: should we get better error description from libpng?
		err = EIO;
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
		err = ENOSYS;
		goto err2;
	}

	res = GP_ContextAlloc(w, h, pixel_type);

	if (res == NULL) {
		err = ENOMEM;
		goto err2;
	}

	if (color_type == PNG_COLOR_TYPE_GRAY)
		png_set_packswap(png);

	uint32_t y;
	
	/* start the actuall reading */
	for (y = 0; y < h; y++) {
		png_bytep row = GP_PIXEL_ADDR(res, 0, y);
		png_read_rows(png, &row, NULL, 1);

		if (GP_ProgressCallbackReport(callback, y, h, w)) {
			GP_DEBUG(1, "Operation aborted");
			err = ECANCELED;
			goto err3;
		}
			
	}
	
	png_destroy_read_struct(&png, &png_info, NULL);

	GP_ProgressCallbackDone(callback);
	
	return res;
err3:
	GP_ContextFree(res);
err2:
	png_destroy_read_struct(&png, png_info ? &png_info : NULL, NULL);
err1:
	errno = err;
	return NULL;
}

GP_Context *GP_LoadPNG(const char *src_path, GP_ProgressCallback *callback)
{
	FILE *f;
	GP_Context *res;

	if (GP_OpenPNG(src_path, &f))
		return NULL;

	res = GP_ReadPNG(f, callback);
	
	fclose(f);

	return res;
}

/*
 * Maps gfxprim Pixel Type to the PNG format
 */
static int prepare_png_header(const GP_Context *src, png_structp png,
                              png_infop png_info, int *bit_endian_flag)
{
	int bit_depth, color_type;

	switch (src->pixel_type) {
	case GP_PIXEL_BGR888:
	case GP_PIXEL_RGB888:
		bit_depth = 8;
		color_type = PNG_COLOR_TYPE_RGB;
	break;
	case GP_PIXEL_G1:
		bit_depth = 1;
		color_type = PNG_COLOR_TYPE_GRAY;
	break;
	case GP_PIXEL_G2:
		bit_depth = 2;
		color_type = PNG_COLOR_TYPE_GRAY;
	break;
	case GP_PIXEL_G4:
		bit_depth = 4;
		color_type = PNG_COLOR_TYPE_GRAY;
	break;
	case GP_PIXEL_G8:
		bit_depth = 8;
		color_type = PNG_COLOR_TYPE_GRAY;
	break;
	default:
		return 1;
	break;
	}

	/* If pointers weren't passed, just return it is okay */
	if (png == NULL || png_info == NULL)
		return 0;

	png_set_IHDR(png, png_info, src->w, src->h, bit_depth, color_type,
	             PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT,
		     PNG_FILTER_TYPE_DEFAULT);
	
	/* start the actuall writing */
	png_write_info(png, png_info);
	
	//png_set_packing(png);

	/* prepare for format conversion */
	switch (src->pixel_type) {
	case GP_PIXEL_RGB888:
		png_set_bgr(png);
	break;
	case GP_PIXEL_G1:
	case GP_PIXEL_G2:
	case GP_PIXEL_G4:
		*bit_endian_flag = !src->bit_endian;
	break;
	default:
	break;
	}

	return 0;
}

static int write_png_data(const GP_Context *src, png_structp png,
                          GP_ProgressCallback *callback, int bit_endian_flag)
{
	/* Look if we need to swap data when writing */
	if (bit_endian_flag) {
		switch (src->pixel_type) {
		case GP_PIXEL_G1:
		case GP_PIXEL_G2:
		case GP_PIXEL_G4:
			png_set_packswap(png);
		break;
		default:
			return ENOSYS;
		break;
		}
	}

	unsigned int y;

	for (y = 0; y < src->h; y++) {
		png_bytep row = GP_PIXEL_ADDR(src, 0, y);
		png_write_row(png, row);

		if (GP_ProgressCallbackReport(callback, y, src->h, src->w)) {
			GP_DEBUG(1, "Operation aborted");
			return ECANCELED;
		}
	}

	return 0;
}

int GP_SavePNG(const GP_Context *src, const char *dst_path,
               GP_ProgressCallback *callback)
{
	FILE *f;
	png_structp png;
	png_infop png_info = NULL;
	int err;

	GP_DEBUG(1, "Saving PNG Image '%s'", dst_path);

	if (prepare_png_header(src, NULL, NULL, NULL)) {
		GP_DEBUG(1, "Can't save png with %s pixel type",
		         GP_PixelTypeName(src->pixel_type));
		return ENOSYS;
	}

	f = fopen(dst_path, "wb");

	if (f == NULL) {
		err = errno;
		GP_DEBUG(1, "Failed to open '%s' for writing: %s",
		         dst_path, strerror(errno));
		goto err0;
	}

	png = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);

	if (png == NULL) {
		GP_DEBUG(1, "Failed to allocate PNG write buffer");
		err = ENOMEM;
		goto err2;
	}

	png_info = png_create_info_struct(png);

	if (png_info == NULL) {
		GP_DEBUG(1, "Failed to allocate PNG info buffer");
		err = ENOMEM;
		goto err3;
	}

	if (setjmp(png_jmpbuf(png))) {
		GP_DEBUG(1, "Failed to write PNG file :(");
		//TODO: should we get better error description from libpng?
		err = EIO;
		goto err3;
	}

	png_init_io(png, f);
	
	int bit_endian_flag = 0;
	/* Fill png header and prepare for data */
	prepare_png_header(src, png, png_info, &bit_endian_flag);

	/* Write bitmap buffer */
	if ((err = write_png_data(src, png, callback, bit_endian_flag)))
		goto err3;

	png_write_end(png, png_info);
	png_destroy_write_struct(&png, &png_info);

	if (fclose(f)) {
		err = errno;
		GP_DEBUG(1, "Failed to close file '%s': %s",
		         dst_path, strerror(errno));
		goto err1;
	}

	GP_ProgressCallbackDone(callback);
	
	return 0;
err3:
	png_destroy_write_struct(&png, png_info == NULL ? NULL : &png_info);
err2:
	fclose(f);
err1:
	unlink(dst_path);
err0:
	errno = err;
	return 1;
}

#else

int GP_OpenPNG(const char GP_UNUSED(*src_path),
               FILE GP_UNUSED(**f))
{
	errno = ENOSYS;
	return 1;
}

GP_Context *GP_ReadPNG(FILE GP_UNUSED(*f),
                       GP_ProgressCallback GP_UNUSED(*callback))
{
	errno = ENOSYS;
	return NULL;
}

GP_Context *GP_LoadPNG(const char GP_UNUSED(*src_path),
                       GP_ProgressCallback GP_UNUSED(*callback))
{
	errno = ENOSYS;
	return NULL;
}

int GP_SavePNG(const GP_Context GP_UNUSED(*src),
               const char GP_UNUSED(*dst_path),
               GP_ProgressCallback GP_UNUSED(*callback))
{
	errno = ENOSYS;
	return 1;
}

#endif /* HAVE_LIBPNG */
