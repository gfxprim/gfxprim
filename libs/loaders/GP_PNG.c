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

  PNG image support using libpng.

 */

#include <stdint.h>
#include <inttypes.h>

#include <errno.h>
#include <string.h>

#include "../../config.h"
#include "core/GP_ByteOrder.h"
#include "core/GP_Debug.h"

#include "GP_PNG.h"

#ifdef HAVE_LIBPNG

#include <png.h>

#include "core/GP_BitSwap.h"

int GP_MatchPNG(const void *buf)
{
	return !png_sig_cmp(buf, 0, 8);
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

static void read_data(png_structp png_ptr, png_bytep data, png_size_t len)
{
	GP_IO *io = png_get_io_ptr(png_ptr);

	if (GP_IOFill(io, data, len))
		png_error(png_ptr, "Read Error");
}

static void load_meta_data(png_structp png, png_infop png_info,
                           GP_DataStorage *storage)
{
	double gamma;
	png_uint_32 res_x, res_y, w, h;
	int unit, depth, color_type, interlace_type, compr_method;
	const char *type;

	png_get_IHDR(png, png_info, &w, &h, &depth,
	             &color_type, &interlace_type, &compr_method, NULL);

	GP_DataStorageAddString(storage, NULL, "Interlace Type",
	                        interlace_type_name(interlace_type));


	GP_DataStorageAddInt(storage, NULL, "Width", w);
	GP_DataStorageAddInt(storage, NULL, "Height", h);
	GP_DataStorageAddInt(storage, NULL, "Bit Depth", depth);

	if (color_type & PNG_COLOR_MASK_PALETTE) {
		type = "Palette";
	} else {
		if (color_type & PNG_COLOR_MASK_COLOR)
			type = "RGB";
		else
			type = "Grayscale";
	}

	GP_DataStorageAddString(storage, NULL, "Color Type", type);

	//TODO: To string
	GP_DataStorageAddInt(storage, NULL, "Compression Method", compr_method);

	/* TODO: BOOL ? */
	GP_DataStorageAddString(storage, NULL, "Alpha Channel",
			        color_type & PNG_COLOR_MASK_ALPHA ? "Yes" : "No");

	if (png_get_gAMA(png, png_info, &gamma))
		GP_DataStorageAddInt(storage, NULL, "gamma", gamma * 100000);

	if (png_get_pHYs(png, png_info, &res_x, &res_y, &unit)) {
		GP_DataStorageAddInt(storage, NULL, "X Resolution", res_x);
		GP_DataStorageAddInt(storage, NULL, "Y Resolution", res_y);

		const char *str_unit;

		if (unit == PNG_RESOLUTION_METER)
			str_unit = "Meter";
		else
			str_unit = "Unknown";

		GP_DataStorageAddString(storage, NULL, "Resolution Unit", str_unit);
	}

	png_timep mod_time;

	if (png_get_tIME(png, png_info, &mod_time)) {
		char buf[128];

		snprintf(buf, sizeof(buf), "%4i:%02i:%02i %02i:%02i:%02i",
		         mod_time->year, mod_time->month, mod_time->day,
			 mod_time->hour, mod_time->minute, mod_time->second);

		GP_DataStorageAddString(storage, NULL, "Date Time", buf);
	}

	png_textp text_ptr;
	int text_cnt;

	if (png_get_text(png, png_info, &text_ptr, &text_cnt)) {
		int i;
		char buf[128];

		for (i = 0; i < text_cnt; i++) {

			if (text_ptr[i].compression != PNG_TEXT_COMPRESSION_NONE)
				continue;

			snprintf(buf, sizeof(buf), "Text %03i", i);
			GP_DataStorageAddString(storage, NULL, buf, text_ptr[i].text);
		}
	}
}

int GP_ReadPNGEx(GP_IO *io, GP_Pixmap **img,
                 GP_DataStorage *storage, GP_ProgressCallback *callback)
{
	png_structp png;
	png_infop png_info = NULL;
	png_uint_32 w, h;
	int depth, color_type, interlace_type;
	GP_PixelType pixel_type = GP_PIXEL_UNKNOWN;
	GP_Pixmap *res = NULL;
	int err, passes = 1;
	double gamma;

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

	png_set_read_fn(png, io, read_data);
	png_set_sig_bytes(png, 0);
	png_read_info(png, png_info);

	if (storage)
		load_meta_data(png, png_info,  storage);

	if (!img)
		goto exit;

	png_get_IHDR(png, png_info, &w, &h, &depth,
	             &color_type, &interlace_type, NULL, NULL);

	png_get_gAMA(png, png_info, &gamma);

	GP_DEBUG(2, "Interlace=%s%s %s PNG%s size %ux%u depth %i gamma %.2lf",
	         interlace_type_name(interlace_type),
	         color_type & PNG_COLOR_MASK_PALETTE ? " pallete" : "",
	         color_type & PNG_COLOR_MASK_COLOR ? "color" : "gray",
		 color_type & PNG_COLOR_MASK_ALPHA ? " with alpha channel" : "",
		 (unsigned int)w, (unsigned int)h, depth, gamma);

	if (interlace_type == PNG_INTERLACE_ADAM7)
		passes = png_set_interlace_handling(png);

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
#ifdef GP_PIXEL_G16
		case 16:
			pixel_type = GP_PIXEL_G16;
		break;
#endif
		}
	break;
	case PNG_COLOR_TYPE_GRAY | PNG_COLOR_MASK_ALPHA:
		switch (depth) {
		case 8:
			pixel_type = GP_PIXEL_GA88;
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
	case PNG_COLOR_TYPE_RGB | PNG_COLOR_MASK_ALPHA:

		png_set_bgr(png);
		png_set_swap_alpha(png);

		switch (depth) {
		case 8:
			pixel_type = GP_PIXEL_RGBA8888;
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

		png_read_update_info(png, png_info);

		png_get_IHDR(png, png_info, &w, &h, &depth,
		             &color_type, NULL, NULL, NULL);

		if (color_type & PNG_COLOR_MASK_ALPHA) {
			pixel_type = GP_PIXEL_RGBA8888;
			png_set_swap_alpha(png);
		} else {
			pixel_type = GP_PIXEL_RGB888;
		}
	break;
	}

	if (pixel_type == GP_PIXEL_UNKNOWN) {
		GP_DEBUG(1, "Unimplemented png format");
		err = ENOSYS;
		goto err2;
	}

	res = GP_PixmapAlloc(w, h, pixel_type);

	if (res == NULL) {
		err = ENOMEM;
		goto err2;
	}

	if (color_type == PNG_COLOR_TYPE_GRAY && depth < 8)
		png_set_packswap(png);

#if __BYTE_ORDER == __LITTLE_ENDIAN
	/*
	 * PNG stores 16 bit values in big endian, turn
	 * on conversion to little endian if needed.
	 */
	if (depth > 8) {
		GP_DEBUG(1, "Enabling byte swap for bpp = %u", depth);
		png_set_swap(png);
	}
#endif

	uint32_t y;
	int p;

	/*
	 * Do the actuall reading.
	 *
	 * The passes are needed for adam7 interlacing.
	 */
	for (p = 0; p < passes; p++) {
		for (y = 0; y < h; y++) {
			png_bytep row = GP_PIXEL_ADDR(res, 0, y);
			png_read_row(png, row, NULL);

			if (GP_ProgressCallbackReport(callback, y + h * p, h * passes, w)) {
				GP_DEBUG(1, "Operation aborted");
				err = ECANCELED;
				goto err3;
			}
		}
	}

exit:
	png_destroy_read_struct(&png, &png_info, NULL);

	GP_ProgressCallbackDone(callback);

	if (img)
		*img = res;

	return 0;
err3:
	GP_PixmapFree(res);
err2:
	png_destroy_read_struct(&png, png_info ? &png_info : NULL, NULL);
err1:
	errno = err;
	return 1;
}

static GP_PixelType save_ptypes[] = {
	GP_PIXEL_BGR888,
	GP_PIXEL_RGB888,
	GP_PIXEL_G1,
	GP_PIXEL_G2,
	GP_PIXEL_G4,
	GP_PIXEL_G8,
#ifdef GP_PIXEL_G16
	GP_PIXEL_G16,
#endif
	GP_PIXEL_RGBA8888,
	GP_PIXEL_UNKNOWN,
};

/*
 * Maps gfxprim Pixel Type to the PNG format
 */
static int prepare_png_header(const GP_Pixmap *src, png_structp png,
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
#ifdef GP_PIXEL_G16
	case GP_PIXEL_G16:
		bit_depth = 16;
		color_type = PNG_COLOR_TYPE_GRAY;
	break;
#endif
	case GP_PIXEL_RGBA8888:
		bit_depth = 8;
		color_type =  PNG_COLOR_TYPE_RGB | PNG_COLOR_MASK_ALPHA;
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

	/* prepare for format conversions */
	switch (src->pixel_type) {
	case GP_PIXEL_RGB888:
		png_set_bgr(png);
	break;
	case GP_PIXEL_RGBA8888:
		png_set_bgr(png);
		png_set_swap_alpha(png);
	break;
	case GP_PIXEL_G1:
	case GP_PIXEL_G2:
	case GP_PIXEL_G4:
		*bit_endian_flag = !src->bit_endian;
	break;
	default:
	break;
	}

#if __BYTE_ORDER ==  __LITTLE_ENDIAN
	/*
	 * PNG stores 16 bit values in big endian, turn
	 * on conversion to little endian if needed.
	 */
	if (bit_depth > 8) {
		GP_DEBUG(1, "Enabling byte swap for bpp = %u", bit_depth);
		png_set_swap(png);
	}
#endif

	return 0;
}

static int write_png_data(const GP_Pixmap *src, png_structp png,
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

static void write_data(png_structp png_ptr, png_bytep data, png_size_t len)
{
	GP_IO *io = png_get_io_ptr(png_ptr);

	if (GP_IOWrite(io, data, len) != (ssize_t)len)
		png_error(png_ptr, "Write Error");
}

static void flush_data(png_structp png_ptr)
{
	(void)png_ptr;
}

int GP_WritePNG(const GP_Pixmap *src, GP_IO *io,
                GP_ProgressCallback *callback)
{
	png_structp png;
	png_infop png_info = NULL;
	int err;

	GP_DEBUG(1, "Writing PNG Image to I/O (%p)", io);

	if (prepare_png_header(src, NULL, NULL, NULL)) {
		GP_DEBUG(1, "Can't save png with %s pixel type",
		         GP_PixelTypeName(src->pixel_type));
		errno = ENOSYS;
		return 1;
	}

	png = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);

	if (png == NULL) {
		GP_DEBUG(1, "Failed to allocate PNG write buffer");
		errno = ENOMEM;
		return 1;
	}

	png_info = png_create_info_struct(png);

	if (png_info == NULL) {
		GP_DEBUG(1, "Failed to allocate PNG info buffer");
		err = ENOMEM;
		goto err;
	}

	if (setjmp(png_jmpbuf(png))) {
		GP_DEBUG(1, "Failed to write PNG file :(");
		//TODO: should we get better error description from libpng?
		err = EIO;
		goto err;
	}

	png_set_write_fn(png, io, write_data, flush_data);

	int bit_endian_flag = 0;
	/* Fill png header and prepare for data */
	prepare_png_header(src, png, png_info, &bit_endian_flag);

	/* Write bitmap buffer */
	if ((err = write_png_data(src, png, callback, bit_endian_flag)))
		goto err;

	png_write_end(png, png_info);
	png_destroy_write_struct(&png, &png_info);

	GP_ProgressCallbackDone(callback);
	return 0;
err:
	png_destroy_write_struct(&png, png_info == NULL ? NULL : &png_info);
	errno = err;
	return 1;
}

#else

int GP_MatchPNG(const void GP_UNUSED(*buf))
{
	errno = ENOSYS;
	return 1;
}

int GP_ReadPNGEx(GP_IO GP_UNUSED(*io), GP_Pixmap GP_UNUSED(**img),
                 GP_DataStorage GP_UNUSED(*storage),
                 GP_ProgressCallback GP_UNUSED(*callback))
{
	errno = ENOSYS;
	return 1;
}

int GP_WritePNG(const GP_Pixmap *src, GP_IO GP_UNUSED(*io),
                GP_ProgressCallback *callback)
{
	errno = ENOSYS;
	return 1;
}

#endif /* HAVE_LIBPNG */

GP_Pixmap *GP_LoadPNG(const char *src_path, GP_ProgressCallback *callback)
{
	return GP_LoaderLoadImage(&GP_PNG, src_path, callback);
}

int GP_SavePNG(const GP_Pixmap *src, const char *dst_path,
               GP_ProgressCallback *callback)
{
	return GP_LoaderSaveImage(&GP_PNG, src, dst_path, callback);
}

GP_Pixmap *GP_ReadPNG(GP_IO *io, GP_ProgressCallback *callback)
{
	return GP_LoaderReadImage(&GP_PNG, io, callback);
}

int GP_LoadPNGEx(const char *src_path, GP_Pixmap **img,
                 GP_DataStorage *storage, GP_ProgressCallback *callback)
{
	return GP_LoaderLoadImageEx(&GP_PNG, src_path, img, storage, callback);
}

GP_Loader GP_PNG = {
#ifdef HAVE_LIBPNG
	.Read = GP_ReadPNGEx,
	.Write = GP_WritePNG,
	.save_ptypes = save_ptypes,
#endif
	.Match = GP_MatchPNG,

	.fmt_name = "Portable Network Graphics",
	.extensions = {"png", NULL},
};
