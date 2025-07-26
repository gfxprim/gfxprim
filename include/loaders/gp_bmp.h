// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2009-2020 Cyril Hrubis <metan@ucw.cz>
 */

/**
 * @file gp_bmp.h
 * @brief BMP image loader.
 *
 * Internal BMP loader functions, you shouldn't need to touch these unless you
 * need to parse file formats that embedds a BMP images with incomplete header
 * such as ICO file format.
 */

#ifndef GP_BMP_H
#define GP_BMP_H

/**
 * @brief A BMP header.
 *
 * Sometimes called a DIB header as well.
 */
struct gp_bmp_info_header {
	/** @brief Offset to image data */
	uint32_t pixel_offset;

	/** @brief Offset to the start of the info header */
	uint32_t header_offset;

	/** @brief Header size (palette is on offset header_size + 14) */
	uint32_t header_size;

	/**
	 * @brief Image width in pixels.
	 */
	int32_t w;
	/**
	 * @brief Image height in pixels.
	 *
	 * If negative image is top-down (bottom-up is default)
	 */
	int32_t h;

	uint16_t bpp;
	uint32_t compress_type;
	/**
	 * @brief Number of palette colors.
	 *
	 * If zero image uses whole range (2^bpp colors)
	 */
	uint32_t palette_colors;
	/** @brief Red mask for bitfields compression */
	uint32_t R_mask;
	/** @brief Green mask for bitfields compression */
	uint32_t G_mask;
	/** @brief Blue mask for bitfields compression */
	uint32_t B_mask;
	/** @brief Alpha mask for bitfields compression */
	uint32_t A_mask;
};

/**
 * @brief BMP compression type.
 */
enum gp_bmp_compress {
	/** @brief Uncompressed. */
	COMPRESS_RGB            = 0,
	/** @brief Run-length encoded bitmap. */
	COMPRESS_RLE8           = 1,
	/** @brief Run-length encoded bitmap. */
	COMPRESS_RLE4           = 2,
	/** @brief Bitfield for each channel. */
	COMPRESS_BITFIELDS      = 3,
	/** @brief Only for printers. */
	COMPRESS_JPEG           = 4,
	/** @brief Only for printers. */
	COMPRESS_PNG            = 5,
	/** @brief Bitfields for each channel with alpha. */
	COMPRESS_ALPHABITFIELDS = 6,
	/** @brief Terminating entry. */
	COMPRESS_MAX = COMPRESS_ALPHABITFIELDS,
};

/**
 * @brief BMP header size.
 */
enum gp_bmp_info_header_size {
	/** @brief Old OS/2 format + win 3.0 */
	BITMAPCOREHEADER  = 12,
	/** @brief OS/2 */
	BITMAPCOREHEADER2 = 64,
	/** @brief Most common */
	BITMAPINFOHEADER  = 40,
	/** @brief Undocummented */
	BITMAPINFOHEADER2 = 52,
	/** @brief Undocummented */
	BITMAPINFOHEADER3 = 56,
	/** @brief Adds color space + gamma - win 95/NT4 */
	BITMAPINFOHEADER4 = 108,
	/** @brief Adds ICC color profiles win 98+ */
	BITMAPINFOHEADER5 = 124,
};

/**
 * @brief Attempts to parse BMP info header at current offset in the file IO.
 *
 * Note that pixel_offset is not set by this function since the offset to the
 * pixels is stored in the bitmap file header. This offset must be filled in by
 * the called before the header is passed to gp_bmp_read_pixels() function.
 *
 * @param io An io stream with an BMP info header at current offset.
 * @param header BMP info header to be filled in.
 *
 * @return Zero on success and errno on a failure.
 */
int gp_bmp_read_info_header(gp_io *io, struct gp_bmp_info_header *header);

/**
 * @brief Returns a pixel type suitable for a BMP info header.
 *
 * Returns a pixel type suitable for an pixmap that could be later passed to
 * the gp_bmp_read_pixels().
 *
 * @param header BMP info header.
 *
 * @return A pixel type.
 */
gp_pixel_type gp_bmp_pixel_type(struct gp_bmp_info_header *header);

/**
 * @brief Returns palette size in bytes.
 *
 * @param header BMP info header structure.
 *
 * @return Palette size in bytes.
 */
uint32_t gp_bmp_palette_size(struct gp_bmp_info_header *header);

/**
 * @brief Reads a pixel data.
 *
 * @param io An io stream.
 * @param header BMP info header.
 * @param pixmap Pre-allocated pixmap of correct size and pixel type.
 * @param callback A progress callback.
 *
 * @return Zero on success and errno on a failure.
 */
int gp_bmp_read_pixels(gp_io *io, struct gp_bmp_info_header *header,
                       gp_pixmap *pixmap, gp_progress_cb *callback);

#endif /* GP_BMP_H */
