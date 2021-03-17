// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2009-2014 Cyril Hrubis <metan@ucw.cz>
 */

/*

  BMP loader and writer.

  Thanks Wikipedia for excellent format specification.

 */

#include <stdint.h>
#include <inttypes.h>

#include <ctype.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>

#include <core/gp_debug.h>
#include <core/gp_pixel.h>
#include <core/gp_get_put_pixel.h>
#include <core/gp_temp_alloc.h>

#include <loaders/gp_line_convert.h>
#include <loaders/gp_loaders.gen.h>

#include <loaders/gp_bmp.h>

#define BMP_HEADER_OFFSET  0x0a       /* info header offset - 4 bytes */

static const char *bmp_compress_names[] = {
	"RGB",
	"RLE8",
	"RLE4",
	"BITFIELDS",
	"JPEG",
	"PNG",
	"ALPHABITFIELDS",
};

static const char *bmp_compress_name(uint32_t compress)
{
	if (compress >= COMPRESS_MAX)
		return "Unknown";

	return bmp_compress_names[compress];
}

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

static uint32_t get_palette_size(struct gp_bmp_info_header *header)
{

	if (header->palette_colors)
		return header->palette_colors;

	return (1 << header->bpp);
}

static uint32_t get_palette_pixel_size(struct gp_bmp_info_header *header)
{
	switch (header->header_size) {
	case BITMAPCOREHEADER:
		return 3;
	break;
	default:
		return 4;
	break;
	}
}

uint32_t gp_bmp_palette_size(struct gp_bmp_info_header *header)
{
	return get_palette_pixel_size(header) * get_palette_size(header);
}

static int read_bitfields(gp_io *io, struct gp_bmp_info_header *header)
{
	uint16_t bitfields[] = {
		GP_IO_L4, /* red mask */
		GP_IO_L4, /* green mask */
		GP_IO_L4, /* blue mask */
		GP_IO_END
	};

	if (gp_io_readf(io, bitfields,
	                &header->R_mask,
	                &header->G_mask,
	                &header->B_mask) != 3) {
		GP_DEBUG(1, "Failed to read BITFIELDS");
		return EIO;
	}

	header->A_mask = 0;

	GP_DEBUG(1, "BITFIELDS R=0x%08x, G=0x%08x, B=0x%08x",
	         header->R_mask, header->G_mask, header->B_mask);

	return 0;
}

static int read_alphabitfields(gp_io *io, struct gp_bmp_info_header *header)
{
	uint16_t abitfields[] = {
		GP_IO_L4, /* red mask */
		GP_IO_L4, /* green mask */
		GP_IO_L4, /* blue mask */
		GP_IO_L4, /* alpha mask */
		GP_IO_END
	};

	if (gp_io_readf(io, abitfields,
	                &header->R_mask,
	                &header->G_mask,
	                &header->B_mask,
	                &header->A_mask) != 4) {
		GP_DEBUG(1, "Failed to read BITFIELDS");
		return EIO;
	}

	GP_DEBUG(1, "BITFILES R=0x%08x, G=0x%08x, B=0x%08x, A=0x%08x",
	         header->R_mask, header->G_mask, header->B_mask,
		 header->A_mask);

	return 0;
}

static int read_bmp_info_header(gp_io *io, struct gp_bmp_info_header *header)
{
	uint16_t nr_planes;

	uint16_t bmp_info_header[] = {
		GP_IO_L4, /* width */
		GP_IO_L4, /* height */
		GP_IO_L2, /* number of planes */
		GP_IO_L2, /* bpp */
		GP_IO_L4, /* compression type */
		GP_IO_IGN | 12, /* bitmap size in bytes, resolution */
		GP_IO_L4, /* palette colors */
		GP_IO_I4, /* number of significant colors */
		GP_IO_END
	};

	if (gp_io_readf(io, bmp_info_header,
	                &header->w, &header->h, &nr_planes, &header->bpp,
		        &header->compress_type, &header->palette_colors) != 8) {

		GP_DEBUG(1, "Failed to read bitmap info header");
		return EIO;
	}

	/* This must be 1 according to specs */
	if (nr_planes != 1)
		GP_WARN("Number of planes %"PRId16" should be 1", nr_planes);

	GP_DEBUG(2, "Have BMP bitmap size %"PRId32"x%"PRId32" %"PRIu16" "
	            "bpp, %"PRIu32" pallete colors, '%s' compression",
		    header->w, header->h, header->bpp,
	            get_palette_size(header),
	            bmp_compress_name(header->compress_type));

	switch (header->compress_type) {
	case COMPRESS_BITFIELDS:
		switch (header->header_size) {
		case BITMAPINFOHEADER:
		case BITMAPINFOHEADER2:
			return read_bitfields(io, header);
		default:
			/* Alpha is default in BITMAPINFOHEADER3 and newer */
			return read_alphabitfields(io, header);
		}
	/* Only in BITMAPINFOHEADER */
	case COMPRESS_ALPHABITFIELDS:
		if (header->header_size != BITMAPINFOHEADER)
			GP_DEBUG(1, "Unexpected ALPHABITFIELDS in %s",
			         bitmap_header_size_name(header->header_size));
		return read_alphabitfields(io, header);
	}

	return 0;
}

static int read_bitmap_core_header(gp_io *io, struct gp_bmp_info_header *header)
{
	int16_t nr_planes, w, h;

	uint16_t bmp_core_header[] = {
		GP_IO_L2, /* width */
		GP_IO_L2, /* height */
		GP_IO_L2, /* number of planes */
		GP_IO_L2, /* bpp */
		//GP_IO_I4,//TODO read till 12?
		GP_IO_END
	};

	if (gp_io_readf(io, bmp_core_header, &w, &h,
	                &nr_planes, &header->bpp) != 4) {
		GP_DEBUG(1, "Failed to read bitmap core header");
		return EIO;
	}

	header->w = w;
	header->h = h;
	header->compress_type = COMPRESS_RGB;
	header->palette_colors = 0;

	/* This must be 1 according to specs */
	if (nr_planes != 1)
		GP_DEBUG(1, "Number of planes is %"PRId16" should be 1",
		            nr_planes);

	GP_DEBUG(2, "Have BMP bitmap size %"PRId32"x%"PRId32" %"PRIu16" bpp",
	            header->h, header->w, header->bpp);

	return 0;
}

static int read_bitmap_file_header(gp_io *io, struct gp_bmp_info_header *header)
{
	uint16_t bmp_header[] = {
		'B',
		'M',
		GP_IO_IGN | (4 + 2 + 2), /* 4 bytes filesize + 4 bytes reserved */
		GP_IO_L4, /* offset to pixel data */
		GP_IO_END,
	};

	if (gp_io_readf(io, bmp_header, &header->pixel_offset,
	               &header->header_size) != 4) {
		GP_DEBUG(1, "Failed to read bitmap file header");
		//TODO: EIO vs EINVAL
		return EIO;
	}

	return 0;
}

int gp_bmp_read_info_header(gp_io *io, struct gp_bmp_info_header *header)
{
	int err;

	uint16_t info_header_size[] = {
		GP_IO_L4, /* info header size */
		GP_IO_END,
	};

	header->header_offset = gp_io_tell(io);

	if (gp_io_readf(io, info_header_size, &header->header_size) != 1) {
		GP_DEBUG(1, "Failed to read info header size");
		return EIO;
	}

	GP_DEBUG(2, "BMP header type '%s'",
	            bitmap_header_size_name(header->header_size));

	switch (header->header_size) {
	case BITMAPCOREHEADER:
		err = read_bitmap_core_header(io, header);
	break;
	case BITMAPCOREHEADER2:
		return ENOSYS;
	/* The bitmap core header only adds filelds to the end of the header */
	case BITMAPINFOHEADER:
	case BITMAPINFOHEADER2:
	case BITMAPINFOHEADER3:
	case BITMAPINFOHEADER4:
		err = read_bmp_info_header(io, header);
	break;
	default:
		GP_DEBUG(1, "Unknown header type, continuing anyway");
		err = read_bmp_info_header(io, header);
	break;
	};

	return err;
}

static int read_bitmap_header(gp_io *io, struct gp_bmp_info_header *header)
{
	int ret;

	ret = read_bitmap_file_header(io, header);
	if (ret)
		return ret;

	return gp_bmp_read_info_header(io, header);
}

/*
 * Reads palette, the format is R G B X, each one byte.
 */
static int read_bitmap_palette(gp_io *io, struct gp_bmp_info_header *header,
                               gp_pixel *palette, uint32_t palette_colors)
{
	uint32_t palette_offset = header->header_size + header->header_offset;
	uint8_t pixel_size = get_palette_pixel_size(header);
	uint32_t i;
	int err;

	GP_DEBUG(2, "Offset to BMP palette is 0x%x (%ubytes) "
	            "pixel size %"PRIu8"bytes",
		    palette_offset, palette_offset, pixel_size);

	if (gp_io_seek(io, palette_offset, GP_SEEK_SET) != palette_offset) {
		err = errno;
		GP_DEBUG(1, "Seek to 0x%02x failed: '%s'",
		            BMP_HEADER_OFFSET, strerror(errno));
		return err;
	}

	size_t palette_size = pixel_size * palette_colors;
	uint8_t *buf = gp_temp_alloc(palette_size);

	if (gp_io_fill(io, buf, palette_size)) {
		GP_DEBUG(1, "Failed to read palette: %s", strerror(errno));
		gp_temp_free(palette_size, buf);
		return EIO;
	}

	for (i = 0; i < palette_colors; i++) {
		unsigned int j = i * pixel_size;

		palette[i] = GP_PIXEL_CREATE_RGB888(buf[j+2], buf[j+1], buf[j]);

		GP_DEBUG(3, "Palette[%"PRIu32"] = [0x%02x, 0x%02x, 0x%02x]", i,
		         GP_PIXEL_GET_R_RGB888(palette[i]),
		         GP_PIXEL_GET_G_RGB888(palette[i]),
		         GP_PIXEL_GET_B_RGB888(palette[i]));
	}

	gp_temp_free(palette_size, buf);
	return 0;
}

static int seek_pixels_offset(gp_io *io, struct gp_bmp_info_header *header)
{
	int err;

	GP_DEBUG(2, "Offset to BMP pixels is 0x%x (%ubytes)",
	            header->pixel_offset, header->pixel_offset);

	if (gp_io_seek(io, header->pixel_offset, GP_SEEK_SET) != header->pixel_offset) {
		err = errno;
		GP_DEBUG(1, "Seek to 0x%02x failed: %s",
		            header->pixel_offset, strerror(err));
		return err;
	}

	return 0;
}

gp_pixel_type gp_bmp_pixel_type(struct gp_bmp_info_header *header)
{
	/* handle bitfields */
	switch (header->compress_type) {
	case COMPRESS_BITFIELDS:
	case COMPRESS_ALPHABITFIELDS:
		return gp_pixel_rgb_match(header->R_mask, header->G_mask,
					  header->B_mask, header->A_mask,
					  header->bpp);
	}

	switch (header->bpp) {
	/* palette formats -> expanded to RGB888 */
	case 1:
	case 2:
	case 4:
	case 8:
	/* RGB888 */
	case 24:
		return GP_PIXEL_RGB888;
	case 16:
#ifdef GP_PIXEL_RGB555
		//TODO: May have 1-bit ALPHA channel for BITMAPINFOHEADER3 and newer
		return GP_PIXEL_RGB555;
#else
	//TODO: Conversion to RGB888?
	break;
#endif
	case 32:
		//TODO: May have ALPHA channel for BITMAPINFOHEADER3 and newer
		return GP_PIXEL_xRGB8888;
	}

	return GP_PIXEL_UNKNOWN;
}

/*
 * Returns four byte aligned row size for palette formats.
 */
static uint32_t bitmap_row_size(struct gp_bmp_info_header *header)
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

static uint8_t get_idx(struct gp_bmp_info_header *header,
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

#include "gp_bmp_rle.h"

static int read_palette(gp_io *io, struct gp_bmp_info_header *header,
                        gp_pixmap *pixmap, gp_progress_cb *callback)
{
	uint32_t palette_size = get_palette_size(header);
	uint32_t row_size = bitmap_row_size(header);
	int32_t y;
	int err;

	gp_temp_alloc_create(tmp, sizeof(gp_pixel) * palette_size + row_size);

	gp_pixel *palette = gp_temp_alloc_arr(tmp, gp_pixel, palette_size);

	if ((err = read_bitmap_palette(io, header, palette, palette_size)))
		goto err;

	if ((err = seek_pixels_offset(io, header)))
		goto err;

	uint8_t *row = gp_temp_alloc_arr(tmp, uint8_t, row_size);

	for (y = 0; y < GP_ABS(header->h); y++) {
		int32_t x;

		if (gp_io_fill(io, row, row_size)) {
			err = errno;
			GP_DEBUG(1, "Failed to read row %"PRId32": %s",
			         y, strerror(errno));
			goto err;
		}

		for (x = 0; x < header->w; x++) {
			uint8_t idx = get_idx(header, row, x);
			gp_pixel p;

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

			gp_putpixel_raw_24BPP(pixmap, x, ry, p);
		}

		if (gp_progress_cb_report(callback, y,
		                              pixmap->h, pixmap->w)) {
			GP_DEBUG(1, "Operation aborted");
			err = ECANCELED;
			goto err;
		}
	}

	gp_progress_cb_done(callback);
err:
	gp_temp_alloc_free(tmp);
	return err;
}

static int read_bitfields_or_rgb(gp_io *io, struct gp_bmp_info_header *header,
                                 gp_pixmap *pixmap,
                                 gp_progress_cb *callback)
{
	uint32_t row_size = header->w * (header->bpp / 8);
	uint32_t row_padd = 0;
	int32_t y;
	int err;

	if ((err = seek_pixels_offset(io, header)))
		return err;

	/* Rows are four byte aligned */
	switch (row_size % 4) {
	case 1:
		row_padd++;
	case 2:
		row_padd++;
	case 3:
		row_padd++;
	case 0:
	break;
	}

	for (y = 0; y < GP_ABS(header->h); y++) {
		int32_t ry;

		if (header->h < 0)
			ry = y;
		else
			ry = GP_ABS(header->h) - 1 - y;

		uint8_t *row = GP_PIXEL_ADDR(pixmap, 0, ry);

		if (gp_io_fill(io, row, row_size)) {
			err = errno;
			GP_DEBUG(1, "Failed to read row %"PRId32": %s",
			         y, strerror(err));
			return err;
		}

		if (row_padd) {
			if (gp_io_seek(io, row_padd, GP_SEEK_CUR) == (off_t)-1) {
				err = errno;
				GP_DEBUG(1, "Failed to seek row %"PRId32": %s",
				         y, strerror(err));
				return err;
			}
		}

		if (gp_progress_cb_report(callback, y,
		                          pixmap->h, pixmap->w)) {
			GP_DEBUG(1, "Operation aborted");
			return ECANCELED;
		}
	}

	gp_progress_cb_done(callback);
	return 0;
}

static void check_palette_size(struct gp_bmp_info_header *header)
{
	if (header->palette_colors > 1u << header->bpp) {
		GP_WARN("Corrupted header bpp=%"PRIu16" palette_size=%"PRIu32
		        ", truncating palette_size to %u",
		        header->bpp, header->palette_colors, 1u << header->bpp);
		header->palette_colors = 0;
	}
}

int gp_bmp_read_pixels(gp_io *io, struct gp_bmp_info_header *header,
                       gp_pixmap *pixmap, gp_progress_cb *callback)
{
	if (header->compress_type == COMPRESS_RLE8) {
		check_palette_size(header);
		return read_RLE8(io, header, pixmap, callback);
	}

	switch (header->bpp) {
	case 1:
	/* I haven't been able to locate 2bpp palette bmp file => not tested */
	case 2:
	case 4:
	case 8:
		check_palette_size(header);
		return read_palette(io, header, pixmap, callback);
	case 16:
	case 24:
	case 32:
		return read_bitfields_or_rgb(io, header, pixmap, callback);
	}

	return ENOSYS;
}

static void fill_metadata(struct gp_bmp_info_header *header,
                          gp_storage *storage)
{
	gp_storage_add_int(storage, NULL, "Width", header->w);
	gp_storage_add_int(storage, NULL, "Height", header->h);
	gp_storage_add_string(storage, NULL, "Compression",
	                      bmp_compress_name(header->compress_type));
	gp_storage_add_string(storage, NULL, "Header Type",
	                        bitmap_header_size_name(header->header_size));
	gp_storage_add_int(storage, NULL, "Bits per Sample",
	                     header->bpp);
	//TODO error check
}

int gp_match_bmp(const void *buf)
{
	return !memcmp(buf, "BM", 2);
}

int gp_read_bmp_ex(gp_io *io, gp_pixmap **img, gp_storage *storage,
                   gp_progress_cb *callback)
{
	struct gp_bmp_info_header header;
	gp_pixel_type pixel_type;
	gp_pixmap *pixmap;
	int err;

	if ((err = read_bitmap_header(io, &header)))
		goto err1;

	if (header.w <= 0 || header.h == 0) {
		GP_WARN("Width and/or Height is not > 0");
		err = EINVAL;
		goto err1;
	}

	if (storage)
		fill_metadata(&header, storage);

	switch (header.compress_type) {
	case COMPRESS_RGB:
	case COMPRESS_BITFIELDS:
	case COMPRESS_ALPHABITFIELDS:
	case COMPRESS_RLE8:
	break;
	default:
		GP_DEBUG(2, "Unknown/Unimplemented compression type");
		err = ENOSYS;
		goto err1;
	}

	if ((pixel_type = gp_bmp_pixel_type(&header)) == GP_PIXEL_UNKNOWN) {
		GP_DEBUG(2, "Unknown pixel type");
		err = ENOSYS;
		goto err1;
	}

	if (!img)
		return 0;

	pixmap = gp_pixmap_alloc(header.w, GP_ABS(header.h), pixel_type);

	if (pixmap == NULL) {
		err = ENOMEM;
		goto err1;
	}

	if ((err = gp_bmp_read_pixels(io, &header, pixmap, callback)))
		goto err2;

	*img = pixmap;

	return 0;
err2:
	gp_pixmap_free(pixmap);
err1:
	errno = err;
	return 1;
}

/*
 * Rows in bmp are four byte aligned.
 */
static uint32_t bmp_align_row_size(uint32_t width_bits)
{
	uint32_t bytes = (width_bits/8) + !!(width_bits%8);

	if (bytes%4)
		bytes += 4 - bytes%4;

	return bytes;
}

/*
 * For uncompressd images
 */
static uint32_t bmp_count_bitmap_size(struct gp_bmp_info_header *header)
{
	return header->h * bmp_align_row_size(header->bpp * header->w);
}

static int bmp_write_header(gp_io *io, struct gp_bmp_info_header *header)
{
	uint32_t bitmap_size = bmp_count_bitmap_size(header);
	uint32_t file_size = bitmap_size + header->header_size + 14;

	uint16_t bitmap_header[] = {
		'B', 'M',               /* signature */
		GP_IO_L4,               /* pixels offset */
		0x00, 0x00, 0x00, 0x00, /* reserved */
		GP_IO_L4,               /* file size */
		GP_IO_END,
	};

	if (gp_io_writef(io, bitmap_header, file_size, header->pixel_offset))
		return EIO;

	uint16_t bmp_info_header[] = {
		GP_IO_L4,   /* header size */
		GP_IO_L4,   /* width */
		GP_IO_L4,   /* height */
		0x01, 0x00, /* nr planes, always 1 */
		GP_IO_L2,   /* bpp */
		GP_IO_L4,   /* compression type */
		GP_IO_L4,   /* bitmap size */
		0, 0, 0, 0, /* X PPM */
		0, 0, 0, 0, /* Y PPM */
		GP_IO_L4,   /* palette colors */
		0, 0, 0, 0, /* important palette colors */
		GP_IO_END,
	};

	if (gp_io_writef(io, bmp_info_header, header->header_size,
	                 header->w, header->h, header->bpp,
	                 header->compress_type, bitmap_size,
	                 header->palette_colors)) {
		return EIO;
	}

	return 0;
}

static gp_pixel_type out_pixel_types[] = {
	GP_PIXEL_RGB888,
	GP_PIXEL_UNKNOWN,
};

static int bmp_fill_header(const gp_pixmap *src, struct gp_bmp_info_header *header)
{
	gp_pixel_type out_pix;

	out_pix = gp_line_convertible(src->pixel_type, out_pixel_types);

	if (out_pix == GP_PIXEL_UNKNOWN) {
		GP_DEBUG(1, "Unsupported pixel type %s",
		         gp_pixel_type_name(src->pixel_type));
		return ENOSYS;
	}

	header->bpp = 24;
	header->w = src->w;
	header->h = src->h;

	/* Most common 40 bytes Info Header */
	header->header_size = BITMAPINFOHEADER;

	/* No compression or palette */
	header->palette_colors = 0;
	header->compress_type = COMPRESS_RGB;

	/* image data follows the header */
	header->pixel_offset = header->header_size + 14;

	return 0;
}

static int bmp_write_data(gp_io *io, const gp_pixmap *src,
                          gp_progress_cb *callback)
{
	int y;
	uint32_t padd_len = 0;
	char padd[3] = {0};
	uint8_t tmp[3 * src->w];
	gp_line_convert convert;

	convert = gp_line_convert_get(src->pixel_type, GP_PIXEL_RGB888);

	if (src->bytes_per_row%4)
		padd_len = 4 - src->bytes_per_row%4;

	for (y = src->h - 1; y >= 0; y--) {
		void *row = GP_PIXEL_ADDR(src, 0, y);

		if (src->pixel_type != GP_PIXEL_RGB888) {
			convert(row, tmp, src->w);
			row = tmp;
		}

		if (gp_io_write(io, row, src->bytes_per_row) != src->bytes_per_row)
			return EIO;

		/* write padding */
		if (padd_len) {
			if (gp_io_write(io, padd, padd_len) != padd_len)
				return EIO;
		}

		if (gp_progress_cb_report(callback, y, src->h, src->w)) {
			GP_DEBUG(1, "Operation aborted");
			return ECANCELED;
		}
	}

	gp_progress_cb_done(callback);

	return 0;
}

int gp_write_bmp(const gp_pixmap *src, gp_io *io, gp_progress_cb *callback)
{
	struct gp_bmp_info_header header;
	int err;

	GP_DEBUG(1, "Writing BMP to I/O (%p)", io);

	if ((err = bmp_fill_header(src, &header)))
		goto err;

	if ((err = bmp_write_header(io, &header)))
		goto err;

	if ((err = bmp_write_data(io, src, callback)))
		goto err;

	return 0;
err:
	errno = err;
	return 1;
}

const struct gp_loader gp_bmp = {
	.read = gp_read_bmp_ex,
	.write = gp_write_bmp,
	.save_ptypes = out_pixel_types,
	.match = gp_match_bmp,

	.fmt_name = "BMP",
	.extensions = {"bmp", "dib", NULL},
};
