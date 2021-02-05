// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2009-2020 Cyril Hrubis <metan@ucw.cz>
 */

/*

   Internal BMP loader functions, you shouldn't need to touch these unless you
   need to parse file formats that embedds a BMP images with incomplete header
   such as ICO file format.

 */

#ifndef GP_BMP_H__
#define GP_BMP_H__

/*
 * Sometimes called a DIB header as well.
 */
struct gp_bmp_info_header {
	/* Offset to image data */
	uint32_t pixel_offset;

	/* Offset to the start of the info header */
	uint32_t header_offset;

	/* Header size (palette is on offset header_size + 14) */
	uint32_t header_size;

	/*
	 * Image size in pixels.
	 *
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
	/*
	 * RGBA masks for bitfields compression
	 */
	uint32_t R_mask;
	uint32_t G_mask;
	uint32_t B_mask;
	uint32_t A_mask;
};

enum gp_bmp_compress {
	COMPRESS_RGB            = 0, /* uncompressed */
	COMPRESS_RLE8           = 1, /* run-length encoded bitmap */
	COMPRESS_RLE4           = 2, /* run-length encoded bitmap */
	COMPRESS_BITFIELDS      = 3, /* bitfield for each channel */
	COMPRESS_JPEG           = 4, /* only for printers */
	COMPRESS_PNG            = 5, /* only for printers */
	COMPRESS_ALPHABITFIELDS = 6,
	COMPRESS_MAX = COMPRESS_ALPHABITFIELDS,
};

enum gp_bmp_info_header_size {
	BITMAPCOREHEADER  = 12,  /* old OS/2 format + win 3.0             */
	BITMAPCOREHEADER2 = 64,  /* OS/2                                  */
	BITMAPINFOHEADER  = 40,  /* most common                           */
	BITMAPINFOHEADER2 = 52,  /* Undocummented                         */
	BITMAPINFOHEADER3 = 56,  /* Undocummented                         */
	BITMAPINFOHEADER4 = 108, /* adds color space + gamma - win 95/NT4 */
	BITMAPINFOHEADER5 = 124, /* adds ICC color profiles win 98+       */
};

/*
 * @brief Attempts to parse BMP info header at current offset in the file IO.
 *
 * Note that pixel_offset is not set by this function since the offset to the
 * pixels is stored in the bitmap file header. This offset must be filled in by
 * the called before the header is passed to gp_bmp_read_pixels() function.
 *
 * @io     An io stream with an BMP info header at current offset.
 * @header BMP info header to be filled in.
 *
 * @return Returns 0 on success and errno on a failure.
 */
int gp_bmp_read_info_header(gp_io *io, struct gp_bmp_info_header *header);

/*
 * @brief Returns a pixel type suitable for a BMP info header.
 *
 * Returns a pixel type suitable for an pixmap that could be later passed to
 * the gp_bmp_read_pixels().
 *
 * @header BMP info header.
 *
 * @return A pixel type.
 */
gp_pixel_type gp_bmp_pixel_type(struct gp_bmp_info_header *header);

/*
 * @brief Returns palette size in bytes.
 *
 * @header BMP info header structure.
 *
 * @return Palette size in bytes.
 */
uint32_t gp_bmp_palette_size(struct gp_bmp_info_header *header);

/*
 * @brief Reads a pixel data.
 *
 * @io An io stream.
 * @header BMP info header.
 * @pixmap Pre-allocated pixmap of correct size and pixel type.
 * @callback A progress callback.
 *
 * @return Returns 0 on success and errno on a failure.
 */
int gp_bmp_read_pixels(gp_io *io, struct gp_bmp_info_header *header,
                       gp_pixmap *pixmap, gp_progress_cb *callback);

#endif /* GP_BMP_H__ */
