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
 * Copyright (C) 2009-2013 Cyril Hrubis <metan@ucw.cz>                       *
 *                                                                           *
 *****************************************************************************/

/*

  TIFF image support using libtiff.

 */

#include <stdint.h>
#include <inttypes.h>

#include <errno.h>
#include <string.h>

#include "../../config.h"
#include "core/GP_Debug.h"

#include "GP_TIFF.h"

#ifdef HAVE_TIFF

#include <tiffio.h>

#define TIFF_HEADER_LITTLE "II\x2a\0"
#define TIFF_HEADER_BIG    "BB\0\x2a"

int GP_MatchTIFF(const void *buf)
{
	if (!memcmp(buf, TIFF_HEADER_LITTLE, 4))
		return 1;

	if (!memcmp(buf, TIFF_HEADER_BIG, 4))
		return 1;

	return 0;
}

int GP_OpenTIFF(const char *src_path, void **t)
{
	TIFF *tiff = TIFFOpen(src_path, "r");

	if (tiff == NULL)
		return 1;

	*t = tiff;
	return 0;
}

static const char *compression_name(uint16_t compression)
{
	switch (compression) {
	case COMPRESSION_NONE:
		return "None";
	case COMPRESSION_CCITTRLE:
		return "CCITT modified Huffman RLE";
	/* COMPRESSION_CCITTFAX3 == COMPRESSION_CCITT_T4 */
	case COMPRESSION_CCITTFAX3:
		return "CCITT Group 3 fax encoding / CCITT T.4 (TIFF 6 name)";
	/* COMPRESSION_CCITTFAX4 == COMPRESSION_CCITT_T6 */
	case COMPRESSION_CCITTFAX4:
		return "CCITT Group 4 fax encoding / CCITT T.6 (TIFF 6 name)";
	case COMPRESSION_LZW:
		return "LZW";
	case COMPRESSION_OJPEG:
		return "JPEG 6.0";
	case COMPRESSION_JPEG:
		return "JPEG DCT";
	case COMPRESSION_NEXT:
		return "NeXT 2 bit RLE";
	case COMPRESSION_CCITTRLEW:
		return "#1 w/ word alignment";
	case COMPRESSION_PACKBITS:
		return "Macintosh RLE";
	case COMPRESSION_THUNDERSCAN:
		return "ThunderScan RLE";
	}

	return "Unknown";
}

GP_Context *GP_ReadTIFF(void *t, GP_ProgressCallback *callback)
{
	uint32_t w, h, y, tile_w, tile_h, rows_per_strip;
	uint16_t compression, bpp;
	TIFF *tiff = t;
        TIFFRGBAImage img;
	GP_Context *res;
	int err;

	/* all these fields are compulsory in tiff image */
	TIFFGetField(tiff, TIFFTAG_IMAGEWIDTH, &w);
        TIFFGetField(tiff, TIFFTAG_IMAGELENGTH, &h);
	TIFFGetField(tiff, TIFFTAG_COMPRESSION, &compression);
	TIFFGetField(tiff, TIFFTAG_BITSPERSAMPLE, &bpp);

	GP_DEBUG(1, "TIFF image %ux%u compression: %s, bpp: %u",
	         w, h, compression_name(compression), bpp);

	/* If set tiff is saved in tiles */
	if (TIFFGetField(tiff, TIFFTAG_TILEWIDTH, &tile_w) &&
	    TIFFGetField(tiff, TIFFTAG_TILELENGTH, &tile_h)) {
		GP_DEBUG(1, "TIFF is tiled in %ux%u", tile_w, tile_h);
		err = ENOSYS;
		goto err1;
	}

	if (!TIFFGetField(tiff, TIFFTAG_ROWSPERSTRIP, &rows_per_strip)) {
		GP_DEBUG(1, "TIFF is not stored in strips");
		err = ENOSYS;
		goto err1;
	} else {
		GP_DEBUG(1, "TIFF rows_per_strip = %u", rows_per_strip);
	}

	res = GP_ContextAlloc(w, h, GP_PIXEL_xRGB8888);

	if (res == NULL) {
		err = errno;
		GP_WARN("Malloc failed");
		goto err1;
	}

	char emsg[1024];

	if (TIFFRGBAImageBegin(&img, tiff, 0, emsg) != 1) {
		GP_DEBUG(1, "TIFFRGBAImageBegin failed: %s", emsg);
		err = EINVAL;
		goto err2;
	}

	for (y = 0; y < h; y += rows_per_strip) {
		void *row = GP_PIXEL_ADDR(res, 0, y);

		if (TIFFReadRGBAStrip(tiff, y, row) != 1) {
			err = EINVAL;
			goto err2;
		}

		if (GP_ProgressCallbackReport(callback, y, h, w)) {
			GP_DEBUG(1, "Operation aborted");
			err = ECANCELED;
			goto err2;
		}
	}

	TIFFRGBAImageEnd(&img);

	GP_ProgressCallbackDone(callback);
	return res;

err2:
	GP_ContextFree(res);
err1:
	errno = err;
	return NULL;
}

GP_Context *GP_LoadTIFF(const char *src_path, GP_ProgressCallback *callback)
{
	void *t;
	GP_Context *res;

	if (GP_OpenTIFF(src_path, &t))
		return NULL;

	res = GP_ReadTIFF(t, callback);

	TIFFClose(t);

	return res;
}

#else

int GP_MatchTIFF(const void GP_UNUSED(*buf))
{
	errno = ENOSYS;
	return -1;
}

int GP_OpenTIFF(const char GP_UNUSED(*src_path),
                void GP_UNUSED(**t))
{
	errno = ENOSYS;
	return 1;
}

GP_Context *GP_ReadTIFF(void GP_UNUSED(*t),
                        GP_ProgressCallback GP_UNUSED(*callback))
{
	errno = ENOSYS;
	return NULL;
}

GP_Context *GP_LoadTIFF(const char GP_UNUSED(*src_path),
                        GP_ProgressCallback GP_UNUSED(*callback))
{
	errno = ENOSYS;
	return NULL;
}

#endif /* HAVE_TIFF */
