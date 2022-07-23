// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2009-2022 Cyril Hrubis <metan@ucw.cz>
 */

/*

  PNG image support.

 */

#include <stdint.h>
#include <inttypes.h>

#include <errno.h>
#include <string.h>

#include "../../config.h"
#include <core/gp_byte_order.h>
#include <core/gp_debug.h>
#include <core/gp_gamma_correction.h>

#include <loaders/gp_io.h>
#include <loaders/gp_line_convert.h>
#include <loaders/gp_loaders.gen.h>

enum color_types {
	COLOR_MASK_PALETTE = 0x01,
	COLOR_MASK_COLOR = 0x02,
	COLOR_MASK_ALPHA = 0x04,
};

enum interlace_methods {
	INTERLACE_NONE = 0,
	INTERLACE_ADAM7 = 1,
};

static const char *interlace_type_name(uint8_t interlace_method)
{
	switch (interlace_method) {
	case INTERLACE_NONE:
		return "none";
	case INTERLACE_ADAM7:
		return "adam7";
	default:
		return "unknown/invalid";
	}
}

#ifdef HAVE_LIBPNG

#include <png.h>

#include <core/gp_bit_swap.h>

int gp_match_png(const void *buf)
{
	return !png_sig_cmp(buf, 0, 8);
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

#include <loaders/gp_io_zlib.h>

enum comp_methods {
	COMP_METHOD_ZLIB = 0,
};

enum filter_methods {
	FILTER_METHOD_NONE = 0,
	FILTER_METHOD_SUB = 1,
	FILTER_METHOD_UP = 2,
	FILTER_METHOD_AVG = 3,
	FILTER_METHOD_PAETH = 4,
	FILTER_METHOD_LAST = 4,
};

static const char *filter_method_name(uint8_t filter_method)
{
	switch (filter_method) {
	case FILTER_METHOD_NONE:
		return "none";
	case FILTER_METHOD_SUB:
		return "sub";
	case FILTER_METHOD_UP:
		return "up";
	case FILTER_METHOD_AVG:
		return "avg";
	case FILTER_METHOD_PAETH:
		return "paeth";
	default:
		return "unknown/invalid";
	}
}

int gp_match_png(const void *buf)
{
	const char *header = "\x89PNG\r\n\x1a\n";

	return !memcmp(buf, header, 8);
}

struct IHDR_chunk {
	uint32_t size;
	uint32_t width;
	uint32_t height;
	uint8_t bit_depth;
	uint8_t color_type;
	uint8_t compress_method;
	uint8_t filter_method;
	uint8_t interlace_method;
};

static inline uint8_t paeth(int a, int b, int c)
{
	int p = a + b - c;
	int pa = GP_ABS(p - a);
	int pb = GP_ABS(p - b);
	int pc = GP_ABS(p - c);

	if (pa <= pb && pa <= pc)
		return a;

	if (pb <= pc)
		return b;

	return c;
}

static void decode_filter(uint8_t *scanline, uint8_t *prev_scanline,
                          int scanline_len, uint8_t filter_method,
			  uint8_t stride)
{
	int i;

	GP_DEBUG(5, "Scanline filter method %s first pixel [%i,%i,%i]",
	         filter_method_name(filter_method),
	         (int)scanline[0], (int)scanline[1], (int)scanline[2]);

	switch (filter_method) {
	case FILTER_METHOD_NONE:
	break;
	case FILTER_METHOD_SUB:
		for (i = 0; i < scanline_len; i++)
			scanline[i] += scanline[i-stride];
	break;
	case FILTER_METHOD_UP:
		for (i = 0; i < scanline_len; i++)
			scanline[i] += prev_scanline[i];
	break;
	case FILTER_METHOD_AVG:
		for (i = 0; i < scanline_len; i++)
			scanline[i] += (scanline[i-stride] + prev_scanline[i])/2;
	break;
	case FILTER_METHOD_PAETH:
		for (i = 0; i < scanline_len; i++)
			scanline[i] += paeth(scanline[i-stride], prev_scanline[i], prev_scanline[i-stride]);
	break;
	}
}

static int load_rgb888_image(gp_io *zlib_io, struct IHDR_chunk *IHDR, gp_pixmap **ret, gp_progress_cb *callback)
{
	uint32_t x, y;
	uint32_t width = IHDR->width;
	uint32_t scanline_len = 3 * width;
	uint8_t scanline1[scanline_len + 3];
	uint8_t scanline2[scanline_len + 3];
	uint8_t *prev_scanline = scanline1 + 2;
	uint8_t *cur_scanline = scanline2 + 2;
	int err;
	gp_pixmap *res;

	memset(scanline1, 0, scanline_len + 3);
	memset(scanline2, 0, scanline_len + 3);

	res = gp_pixmap_alloc(IHDR->width, IHDR->height, GP_PIXEL_RGB888);
	if (!res)
		return errno;

	for (y = 0; y < IHDR->height; y++) {
		if (gp_io_fill(zlib_io, cur_scanline, scanline_len+1)) {
			GP_DEBUG(1, "Failed to read scanline %u", (unsigned int)y);
			err = errno;
			goto err;
		}

		uint8_t filter_method = cur_scanline[0];

		cur_scanline[0] = 0;

		if (filter_method > FILTER_METHOD_LAST) {
			GP_DEBUG(1, "Invalid filter method %i", (int)filter_method);
			err = EINVAL;
			goto err;
		}

		decode_filter(cur_scanline+1, prev_scanline+1, scanline_len, filter_method, 3);

		uint8_t *row = GP_PIXEL_ADDR(res, 0, y);

		for (x = 0; x < IHDR->width; x++) {
			row[3 * x + 2] = cur_scanline[3 * x + 1];
			row[3 * x + 1] = cur_scanline[3 * x + 2];
			row[3 * x + 0] = cur_scanline[3 * x + 3];
		}

		GP_SWAP(cur_scanline, prev_scanline);

		if (gp_progress_cb_report(callback, y, res->h, res->w)) {
			GP_DEBUG(1, "Operation aborted");
			err = ECANCELED;
			goto err;
		}
	}

	*ret = res;
	return 0;
err:
	gp_pixmap_free(res);
	return err;
}

#define CHUNK_ID_TO_INT(c1, c2, c3, c4) (((uint32_t)c1) |       \
		                         (((uint32_t)c2)<<8) |  \
		                         (((uint32_t)c2)<<16) | \
		                         (((uint32_t)c2)<<24))

#define CHUNK_ID_HEADER_TO_INT(chunk_header) \
	CHUNK_ID_TO_INT((chunk_header)->id[0], \
	                (chunk_header)->id[1], \
	                (chunk_header)->id[2], \
	                (chunk_header)->id[3])

struct chunk_header {
	char id[5];
	uint32_t size;
};

static int next_chunk(gp_io *io, struct chunk_header *chunk_header)
{
	const uint16_t chunk[] = {
		GP_IO_B4,   /* Lenght */
		GP_IO_BYTE, /* Chunk ID */
		GP_IO_BYTE,
		GP_IO_BYTE,
		GP_IO_BYTE,
		GP_IO_END
	};

	if (gp_io_readf(io, chunk, &chunk_header->size,
			&chunk_header->id[0], &chunk_header->id[1],
	                &chunk_header->id[2], &chunk_header->id[3]) != 5) {
		GP_DEBUG(1, "Failed to read next chunk!");
		return EINVAL;
	}

	return 0;
}

/* Creates a single continous IO stream from PNG IDAT chunks */

struct idat_io {
	struct gp_io out_io;
	gp_io *in_io;
	struct chunk_header *chunk_header;
	size_t chunk_read;
};

static ssize_t idat_read(gp_io *self, void *buf, size_t size)
{
	struct idat_io *idat_io = GP_CONTAINER_OF(self, struct idat_io, out_io);
	ssize_t read;
	size_t avail;

	avail = idat_io->chunk_header->size - idat_io->chunk_read;

	if (avail <= 0) {
		/* Skip CRC */
		gp_io_seek(idat_io->in_io, 4, GP_SEEK_CUR);

		int err = next_chunk(idat_io->in_io, idat_io->chunk_header);
		if (err) {
			errno = err;
			return -1;
		}

		switch (CHUNK_ID_HEADER_TO_INT(idat_io->chunk_header)) {
		case CHUNK_ID_TO_INT('I', 'D', 'A', 'T'):
			idat_io->chunk_read = 0;
			avail = idat_io->chunk_header->size - idat_io->chunk_read;
		break;
		default:
			return 0;
		}
	}

	read = gp_io_read(idat_io->in_io, buf, GP_MIN(size, avail));
	if (read > 0)
		idat_io->chunk_read += read;

	return read;
}

static off_t idat_seek(gp_io *self, off_t off, enum gp_seek_whence whence)
{
	(void) self;

	GP_DEBUG(1, "Dummy seek called off=%lu whence=%i", (long unsigned)off, whence);

	return 0;
}

static int load_image(gp_io *io, struct IHDR_chunk *IHDR,
                      struct chunk_header *chunk_header,
                      gp_pixmap **img, gp_progress_cb *callback)
{
	gp_io *zlib_io;
	uint8_t zlib_comp_method;
	uint8_t flags;
	int err = 0;

	zlib_comp_method = gp_io_getb(io);
	flags = gp_io_getb(io);

	GP_DEBUG(1, "Zlib comp method %u flags %u", zlib_comp_method, flags);

	struct idat_io idat_io = {
		.out_io = {
			.read = idat_read,
			.seek = idat_seek,
		},
		.in_io = io,
		.chunk_header = chunk_header,
		.chunk_read = 2,
	};

	zlib_io = gp_io_zlib(&idat_io.out_io, 0);
	if (!zlib_io)
		return errno;

	switch (IHDR->color_type) {
	case COLOR_MASK_COLOR:
		err = load_rgb888_image(zlib_io, IHDR, img, callback);
		if (err)
			goto ret;
	break;
	}

ret:
	gp_io_close(zlib_io);

	return err;
}

int gp_read_png_ex(gp_io *io, gp_pixmap **img,
                   gp_storage GP_UNUSED(*storage),
                   gp_progress_cb *callback)
{
	struct IHDR_chunk IHDR;
	struct chunk_header chunk_header = {};
	int ret, err;

	const uint16_t header[] = {
		0x89,
		'P',
		'N',
		'G',
		'\r',
		'\n',
		0x1a,
		'\n',
		GP_IO_B4,
		'I',
		'H',
		'D',
		'R',
		GP_IO_B4,
		GP_IO_B4,
		GP_IO_BYTE,
		GP_IO_BYTE,
		GP_IO_BYTE,
		GP_IO_BYTE,
		GP_IO_BYTE,
		GP_IO_END,
	};

	ret = gp_io_readf(io, header, &IHDR.size, &IHDR.width, &IHDR.height,
	                  &IHDR.bit_depth, &IHDR.color_type,
	                  &IHDR.compress_method,
	                  &IHDR.filter_method,
			  &IHDR.interlace_method);

	if (ret != GP_ARRAY_SIZE(header) - 1) {
		GP_DEBUG(1, "Failed to read IHDR chunk");
		errno = EIO;
		return 1;
	}

	GP_DEBUG(2, "Interlace=%s%s %s PNG%s size %ux%u depth %i",
	         interlace_type_name(IHDR.interlace_method),
	         IHDR.color_type & COLOR_MASK_PALETTE ? " pallete" : "",
	         IHDR.color_type & COLOR_MASK_COLOR ? "color" : "gray",
		 IHDR.color_type & COLOR_MASK_ALPHA ? " with alpha channel" : "",
		 (unsigned int)IHDR.width, (unsigned int)IHDR.height, IHDR.bit_depth);

	switch (IHDR.compress_method) {
	case COMP_METHOD_ZLIB:
	break;
	default:
		GP_DEBUG(1, "Unknown/invalid compression method %u",
		         (unsigned int)IHDR.compress_method);
		err = EINVAL;
		goto err;
	}

	switch (IHDR.interlace_method) {
	case INTERLACE_NONE:
	break;
	case INTERLACE_ADAM7:
		GP_DEBUG(1, "Adam7 interalce not supported");
		err = ENOSYS;
		goto err;
	break;
	default:
		GP_DEBUG(1, "Unknown/invalid interalce method");
		err = EINVAL;
		goto err;
	break;
	}

	if (IHDR.filter_method != 0) {
		GP_DEBUG(1, "Unknown/invalid filter method");
		err = EINVAL;
		goto err;
	}

	switch (IHDR.color_type) {
	case COLOR_MASK_COLOR:
	break;
	default:
		GP_DEBUG(1, "Unsupported color type");
		err = ENOSYS;
		goto err;
	}

	/* skip CRC and get right to the next chunk */
	gp_io_seek(io, IHDR.size - 13 + 4, GP_SEEK_CUR);

	for (;;) {
		err = next_chunk(io, &chunk_header);
		if (err)
			goto err;

		GP_DEBUG(3, "Have chunk '%s' size %u",
		         chunk_header.id, (unsigned int)chunk_header.size);

		switch (CHUNK_ID_HEADER_TO_INT(&chunk_header)) {
		case CHUNK_ID_TO_INT('I', 'E', 'N', 'D'):
			goto ret;
		case CHUNK_ID_TO_INT('I', 'D', 'A', 'T'):
			err = load_image(io, &IHDR, &chunk_header, img, callback);
			if (err)
				goto err;
		break;
		default:
			GP_DEBUG(1, "Skipping chunk '%s'", chunk_header.id);
			gp_io_seek(io, chunk_header.size, GP_SEEK_CUR);
		}

		/* Skip CRC */
		gp_io_seek(io, 4, GP_SEEK_CUR);
	}

ret:
	if (*img)
		return 0;

	err = ENOSYS;
err:
	errno = err;
	return 1;
}

int gp_write_png(const gp_pixmap GP_UNUSED(*src), gp_io GP_UNUSED(*io),
                gp_progress_cb GP_UNUSED(*callback))
{
	errno = ENOSYS;
	return 1;
}

#endif /* HAVE_LIBPNG */

const gp_loader gp_png = {
#ifdef HAVE_LIBPNG
	.write = gp_write_png,
	.save_ptypes = save_ptypes,
#endif
	.read = gp_read_png_ex,
	.match = gp_match_png,

	.fmt_name = "Portable Network Graphics",
	.extensions = {"png", NULL},
};
