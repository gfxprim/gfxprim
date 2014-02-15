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

  Photoshop PSD thumbnail image loader.

  Written using documentation available freely on the internet.

 */

#include <stdint.h>
#include <inttypes.h>
#include <string.h>
#include <errno.h>

#include "core/GP_Debug.h"
#include "core/GP_Common.h"
#include "GP_JPG.h"
#include "GP_PSD.h"

#define PSD_SIGNATURE "8BPS\x00\x01"
#define PSD_SIGNATURE_LEN 6

int GP_MatchPSD(const void *buf)
{
	return !memcmp(buf, PSD_SIGNATURE, PSD_SIGNATURE_LEN);
}

enum psd_img_res_id {
	PSD_THUMBNAIL_RES40 = 1033,
	PSD_THUMBNAIL_RES50 = 1036,
};

enum thumbnail_fmt {
	PSD_RAW_RGB = 0,
	PSD_JPG_RGB = 1,
};

static const char *thumbnail_fmt_name(uint16_t fmt)
{
	switch (fmt) {
	case PSD_RAW_RGB:
		return "Raw RGB";
	case PSD_JPG_RGB:
		return "JPEG RGB";
	default:
		return "Unknown";
	}
}

static GP_Context *psd_thumbnail50(GP_IO *io, GP_ProgressCallback *callback)
{
	uint32_t fmt, w, h;
	uint16_t bpp, nr_planes;

	uint16_t res_thumbnail_header[] = {
		GP_IO_B4, /* Format */
		GP_IO_B4, /* Width */
		GP_IO_B4, /* Height */
		/*
		 * Widthbytes:
		 * Padded row bytes = (width * bits per pixel + 31) / 32 * 4
		 */
		GP_IO_I4,
		GP_IO_I4, /* Total size: widthbytes * height * planes */
		GP_IO_I4, /* Size after compression */
		GP_IO_B2, /* Bits per pixel = 24 */
		GP_IO_B2, /* Number of planes = 1 */
		GP_IO_END,
	};

	if (GP_IOReadF(io, res_thumbnail_header, &fmt, &w, &h,
	               &bpp, &nr_planes) != 8) {
		GP_DEBUG(1, "Failed to read image thumbnail header");
		return NULL;
	}

	GP_DEBUG(1, "%"PRIu32"x%"PRIu32" format=%s (%"PRIu16") bpp=%"PRIu16
	         " nr_planes=%"PRIu16, w, h, thumbnail_fmt_name(fmt), fmt,
		 bpp, nr_planes);

	if (fmt != PSD_JPG_RGB) {
		GP_DEBUG(1, "Unsupported thumbnail format");
		return NULL;
	}

	return GP_ReadJPG(io, callback);
}

static unsigned int psd_next_img_res_block(GP_IO *io, GP_Context **res,
                                           GP_ProgressCallback *callback)
{
	uint16_t res_id;
	uint32_t res_size;

	uint16_t res_block_header[] = {
		'8', 'B', 'I', 'M', /* Image resource block signature */
		GP_IO_B2,           /* Resource ID */
		//TODO: photoshop pascall string, it's set to 00 00 in most cases though
		GP_IO_I2,
		GP_IO_B4,           /* Resource block size */
		GP_IO_END,
	};

	if (GP_IOReadF(io, res_block_header, &res_id, &res_size) != 7) {
		GP_DEBUG(1, "Failed to read image resource header");
		return 0;
	}

	GP_DEBUG(1, "Image resource id=%"PRIu16" size=%"PRIu32,
	         res_id, res_size);

	switch (res_id) {
	case PSD_THUMBNAIL_RES40:
		GP_DEBUG(1, "Unsupported thumbnail version 4.0");
	break;
	case PSD_THUMBNAIL_RES50:
		*res = psd_thumbnail50(io, callback);
		return 0;
	}

	res_size = GP_ALIGN2(res_size);

	if (GP_IOSeek(io, res_size, GP_IO_SEEK_CUR) == (off_t)-1) {
		GP_DEBUG(1, "Failed skip image resource");
		return 0;
	}

	return res_size + 10;
}

enum psd_color_mode {
	PSD_BITMAP = 0x00,
	PSD_GRAYSCALE = 0x01,
	PSD_INDEXED = 0x02,
	PSD_RGB = 0x03,
	PSD_CMYK = 0x04,
	PSD_MULTICHANNEL = 0x07,
	PSD_DUOTONE = 0x08,
	PSD_LAB = 0x09,
};

static const char *psd_color_mode_name(uint16_t color_mode)
{
	switch (color_mode) {
	case PSD_BITMAP:
		return "Bitmap";
	case PSD_GRAYSCALE:
		return "Grayscale";
	case PSD_INDEXED:
		return "Indexed";
	case PSD_RGB:
		return "RGB";
	case PSD_CMYK:
		return "CMYK";
	case PSD_MULTICHANNEL:
		return "Multichannel";
	case PSD_DUOTONE:
		return "Duotone";
	case PSD_LAB:
		return "Lab";
	default:
		return "Unknown";
	}
}

GP_Context *GP_ReadPSD(GP_IO *io, GP_ProgressCallback *callback)
{
	int err;
	uint32_t w;
	uint32_t h;
	uint16_t depth;
	uint16_t channels;
	uint16_t color_mode;
	uint32_t len, size, read_size = 0;

	uint16_t psd_header[] = {
	        '8', 'B', 'P', 'S',
		0x00, 0x01,         /* Version always 1 */
		GP_IO_IGN | 6,      /* Reserved, should be 0 */
		GP_IO_B2,           /* Channels 1 to 56 */
		GP_IO_B4,           /* Height */
		GP_IO_B4,           /* Width */
		GP_IO_B2,           /* Depth (bits per channel) */
		GP_IO_B2,           /* Color mode */
		GP_IO_B4,           /* Color mode data lenght */
		GP_IO_END
	};

	if (GP_IOReadF(io, psd_header, &channels, &h, &w, &depth,
	               &color_mode, &len) != 13) {
		GP_DEBUG(1, "Failed to read file header");
		err = EIO;
		goto err0;
	}

	GP_DEBUG(1, "Have PSD %"PRIu32"x%"PRIu32" channels=%"PRIu16","
	         " bpp=%"PRIu16" color_mode=%s (%"PRIu16") "
		 " color_mode_data_len=%"PRIu32, w, h, channels,
	         depth, psd_color_mode_name(color_mode), color_mode, len);

	switch (color_mode) {
	case PSD_INDEXED:
	case PSD_DUOTONE:
	break;
	default:
		if (len)
			GP_WARN("Color mode_mode_data_len != 0 (is %"PRIu32")"
			        "for %s (%"PRIu16")", len,
				psd_color_mode_name(color_mode), color_mode);
	}

	/* Seek after the color mode data */
	if (GP_IOSeek(io, len, GP_IO_SEEK_CUR) == (off_t)-1) {
		GP_DEBUG(1, "Failed skip color mode data");
		return NULL;
	}

	if (GP_IOReadB4(io, &len)) {
		GP_DEBUG(1, "Failed to load Image Resource Section Lenght");
		return NULL;
	}

	GP_DEBUG(1, "Image Resource Section length is %x", len);

	GP_Context *res = NULL;

	do {
		size = psd_next_img_res_block(io, &res, callback);

		if (!size)
			return res;

		read_size += size;
	} while (read_size < len);

	errno = ENOSYS;
	return NULL;
err0:
	errno = err;
	return NULL;
}

GP_Context *GP_LoadPSD(const char *src_path, GP_ProgressCallback *callback)
{
	GP_IO *io;
	GP_Context *res;
	int err;

	io = GP_IOFile(src_path, GP_IO_RDONLY);
	if (!io)
		return NULL;

	res = GP_ReadPSD(io, callback);

	err = errno;
	GP_IOClose(io);
	errno = err;

	return res;
}
