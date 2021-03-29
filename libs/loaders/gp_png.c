// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2009-2021 Cyril Hrubis <metan@ucw.cz>
 */

/*

  PNG image support using libpng.

 */

#include <stdint.h>
#include <inttypes.h>

#include <errno.h>
#include <string.h>

#include "../../config.h"
#include <core/gp_byte_order.h>
#include <core/gp_debug.h>
#include <core/gp_gamma_correction.h>

#include <loaders/gp_line_convert.h>
#include <loaders/gp_loaders.gen.h>

#ifdef HAVE_LIBPNG

#include <png.h>

#include <core/gp_bit_swap.h>

int gp_match_png(const void *buf)
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
	gp_io *io = png_get_io_ptr(png_ptr);

	if (gp_io_fill(io, data, len))
		png_error(png_ptr, "Read Error");
}

static void load_meta_data(png_structp png, png_infop png_info,
                           gp_storage *storage)
{
	double gamma;
	png_uint_32 res_x, res_y, w, h;
	int unit, depth, color_type, interlace_type, compr_method;
	const char *type;

	png_get_IHDR(png, png_info, &w, &h, &depth,
	             &color_type, &interlace_type, &compr_method, NULL);

	gp_storage_add_string(storage, NULL, "Interlace Type",
	                        interlace_type_name(interlace_type));


	gp_storage_add_int(storage, NULL, "Width", w);
	gp_storage_add_int(storage, NULL, "Height", h);
	gp_storage_add_int(storage, NULL, "Bit Depth", depth);

	if (color_type & PNG_COLOR_MASK_PALETTE) {
		type = "Palette";
	} else {
		if (color_type & PNG_COLOR_MASK_COLOR)
			type = "RGB";
		else
			type = "Grayscale";
	}

	gp_storage_add_string(storage, NULL, "Color Type", type);

	//TODO: To string
	gp_storage_add_int(storage, NULL, "Compression Method", compr_method);

	/* TODO: BOOL ? */
	gp_storage_add_string(storage, NULL, "Alpha Channel",
			        color_type & PNG_COLOR_MASK_ALPHA ? "Yes" : "No");

	if (png_get_gAMA(png, png_info, &gamma))
		gp_storage_add_int(storage, NULL, "gamma", gamma * 100000);

	if (png_get_pHYs(png, png_info, &res_x, &res_y, &unit)) {
		gp_storage_add_int(storage, NULL, "X Resolution", res_x);
		gp_storage_add_int(storage, NULL, "Y Resolution", res_y);

		const char *str_unit;

		if (unit == PNG_RESOLUTION_METER)
			str_unit = "Meter";
		else
			str_unit = "Unknown";

		gp_storage_add_string(storage, NULL, "Resolution Unit", str_unit);
	}

	png_timep mod_time;

	if (png_get_tIME(png, png_info, &mod_time)) {
		char buf[128];

		snprintf(buf, sizeof(buf), "%4i:%02i:%02i %02i:%02i:%02i",
		         mod_time->year, mod_time->month, mod_time->day,
			 mod_time->hour, mod_time->minute, mod_time->second);

		gp_storage_add_string(storage, NULL, "Date Time", buf);
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
			gp_storage_add_string(storage, NULL, buf, text_ptr[i].text);
		}
	}
}

static int read_bitmap(gp_pixmap *res, gp_progress_cb *callback,
                       png_structp png, int passes)
{
	uint32_t y;
	int p;

	/*
	 * The passes are needed for adam7 interlacing.
	 */
	for (p = 0; p < passes; p++) {
		for (y = 0; y < res->h; y++) {
			png_bytep row = GP_PIXEL_ADDR(res, 0, y);
			png_read_row(png, row, NULL);

			if (gp_progress_cb_report(callback, y + res->h * p, res->h * passes, res->w)) {
				GP_DEBUG(1, "Operation aborted");
				return ECANCELED;
			}
		}
	}

	return 0;
}

static int read_convert_bitmap(gp_pixmap *res, gp_progress_cb *callback,
		               png_structp png, int passes, double gamma)
{
	uint16_t *row;

	if (passes > 1) {
		GP_DEBUG(1, "Interlaced 16 bit PNG not supported");
		return ENOSYS;
	}

	row = malloc(6 * res->w);
	if (!row) {
		GP_DEBUG(1, "Malloc failed :(");
		return ENOMEM;
	}

	unsigned int y;

	if (gamma < 0.01)
		gp_pixmap_set_gamma(res, 2.2);

	for (y = 0; y < res->h; y++) {
		png_read_row(png, (void*)row, NULL);
		uint8_t *rrow = GP_PIXEL_ADDR(res, 0, y);
		unsigned int x = 0;


		if (gamma > 0.1) {
			for (x = 0; x < res->w; x++) {
				rrow[3*x] = row[3 * x]>>8;
				rrow[3*x + 1] = row[3 * x + 1]>>8;
				rrow[3*x + 2] = row[3 * x + 2]>>8;
			}
		} else {
			for (x = 0; x < res->w; x++) {
				rrow[3*x] = gp_linear16_to_gamma8(row[3 * x]);
				rrow[3*x + 1] = gp_linear16_to_gamma8(row[3 * x + 1]);
				rrow[3*x + 2] = gp_linear16_to_gamma8(row[3 * x + 2]);
			}
		}

		if (gp_progress_cb_report(callback, y, res->h, res->w)) {
			GP_DEBUG(1, "Operation aborted");
			free(row);
			return ECANCELED;
		}
	}

	free(row);
	return 0;
}

int gp_read_png_ex(gp_io *io, gp_pixmap **img,
                 gp_storage *storage, gp_progress_cb *callback)
{
	png_structp png;
	png_infop png_info = NULL;
	png_uint_32 w, h;
	int depth, color_type, interlace_type;
	gp_pixel_type pixel_type = GP_PIXEL_UNKNOWN;
	gp_pixmap *res = NULL;
	int err, passes = 1;
	double gamma;
	int convert_16_to_8 = 0;

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
		case 16:
			pixel_type = GP_PIXEL_RGB888;
			convert_16_to_8 = 1;
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

	res = gp_pixmap_alloc(w, h, pixel_type);

	if (res == NULL) {
		err = ENOMEM;
		goto err2;
	}

	if (gamma > 0.1)
		gp_pixmap_set_gamma(res, 1 / gamma);

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
	if (convert_16_to_8) {
		if ((err = read_convert_bitmap(res, callback, png, passes, gamma)))
			goto err3;
	} else {
		if ((err = read_bitmap(res, callback, png, passes)))
			goto err3;
	}

exit:
	png_destroy_read_struct(&png, &png_info, NULL);

	gp_progress_cb_done(callback);

	if (img)
		*img = res;

	return 0;
err3:
	gp_pixmap_free(res);
err2:
	png_destroy_read_struct(&png, png_info ? &png_info : NULL, NULL);
err1:
	errno = err;
	return 1;
}

static gp_pixel_type save_ptypes[] = {
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
static int prepare_png_header(gp_pixel_type ptype, gp_size w, gp_size h, int bit_endian,
                              png_structp png, png_infop png_info,
                              int *bit_endian_flag)
{
	int bit_depth, color_type;

	switch (ptype) {
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
	if (!png || !png_info)
		return 0;

	png_set_IHDR(png, png_info, w, h, bit_depth, color_type,
	             PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT,
		     PNG_FILTER_TYPE_DEFAULT);

	/* start the actuall writing */
	png_write_info(png, png_info);

	//png_set_packing(png);

	/* prepare for format conversions */
	switch (ptype) {
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
		*bit_endian_flag = !bit_endian;
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

static int write_png_data(const gp_pixmap *src, png_structp png,
                          gp_progress_cb *callback, int bit_endian_flag)
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

		if (gp_progress_cb_report(callback, y, src->h, src->w)) {
			GP_DEBUG(1, "Operation aborted");
			return ECANCELED;
		}
	}

	return 0;
}

static int convert_write_png_data(const gp_pixmap *src, gp_pixel_type out_pix,
                                  png_structp png, gp_progress_cb *callback)
{
        uint8_t out[(src->w * gp_pixel_size(out_pix)) / 8 + 1];
        gp_line_convert convert = gp_line_convert_get(src->pixel_type, out_pix);

	unsigned int y;

	for (y = 0; y < src->h; y++) {
		void *in = GP_PIXEL_ADDR(src, 0, y);

		convert(in, out, src->w);
		png_write_row(png, out);

		if (gp_progress_cb_report(callback, y, src->h, src->w)) {
			GP_DEBUG(1, "Operation aborted");
			return ECANCELED;
		}
	}

	return 0;
}

static void write_data(png_structp png_ptr, png_bytep data, png_size_t len)
{
	gp_io *io = png_get_io_ptr(png_ptr);

	if (gp_io_write(io, data, len) != (ssize_t)len)
		png_error(png_ptr, "Write Error");
}

static void flush_data(png_structp png_ptr)
{
	(void)png_ptr;
}

int gp_write_png(const gp_pixmap *src, gp_io *io,
                gp_progress_cb *callback)
{
	png_structp png;
	png_infop png_info = NULL;
	int err;
	gp_pixel_type out_pix = src->pixel_type;

	GP_DEBUG(1, "Writing PNG Image to I/O (%p)", io);

	if (prepare_png_header(src->pixel_type, 0, 0, 0, NULL, NULL, NULL)) {
		out_pix = gp_line_convertible(src->pixel_type, save_ptypes);

		if (out_pix == GP_PIXEL_UNKNOWN) {
			GP_DEBUG(1, "Can't save png with %s pixel type",
			         gp_pixel_type_name(src->pixel_type));
			errno = ENOSYS;
			return 1;
		}
	}

	png = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);

	if (!png) {
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
	prepare_png_header(out_pix, src->w, src->h, src->bit_endian,
	                   png, png_info, &bit_endian_flag);

	/* Write bitmap buffer */
	if (src->pixel_type == out_pix) {
		if ((err = write_png_data(src, png, callback, bit_endian_flag)))
			goto err;
	} else {
		if ((err = convert_write_png_data(src, out_pix, png, callback)))
			goto err;
	}

	png_write_end(png, png_info);
	png_destroy_write_struct(&png, &png_info);

	gp_progress_cb_done(callback);
	return 0;
err:
	png_destroy_write_struct(&png, png_info == NULL ? NULL : &png_info);
	errno = err;
	return 1;
}

#else

int gp_match_png(const void GP_UNUSED(*buf))
{
	errno = ENOSYS;
	return 1;
}

int gp_read_png_ex(GP_IO GP_UNUSED(*io), gp_pixmap GP_UNUSED(**img),
                 gp_storage GP_UNUSED(*storage),
                 gp_progress_cb GP_UNUSED(*callback))
{
	errno = ENOSYS;
	return 1;
}

int gp_write_png(const gp_pixmap *src, GP_IO GP_UNUSED(*io),
                gp_progress_cb *callback)
{
	errno = ENOSYS;
	return 1;
}

#endif /* HAVE_LIBPNG */

const gp_loader gp_png = {
#ifdef HAVE_LIBPNG
	.read = gp_read_png_ex,
	.write = gp_write_png,
	.save_ptypes = save_ptypes,
#endif
	.match = gp_match_png,

	.fmt_name = "Portable Network Graphics",
	.extensions = {"png", NULL},
};
