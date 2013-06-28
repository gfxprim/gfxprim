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

#include "core/GP_Pixel.h"
#include "core/GP_GetPutPixel.h"
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

struct tiff_header {
	/* compulsory tiff data */
	uint32_t w, h;
	uint16_t compress;
	uint16_t bits_per_sample;

	/* either strips or tiles should be set */
	uint32_t rows_per_strip;

	uint32_t tile_w;
	uint32_t tile_h;

	/* pixel type related values */
	uint16_t photometric;
};

static int read_header(TIFF *tiff, struct tiff_header *header)
{
	/* all these fields are compulsory in tiff image */
	if (!TIFFGetField(tiff, TIFFTAG_IMAGEWIDTH, &header->w)) {
		GP_DEBUG(1, "Failed to read Width");
		return EIO;
	}

        if (!TIFFGetField(tiff, TIFFTAG_IMAGELENGTH, &header->h)) {
		GP_DEBUG(1, "Failed to read Height");
		return EIO;
	}

	if (!TIFFGetField(tiff, TIFFTAG_COMPRESSION, &header->compress)) {
		GP_DEBUG(1, "Failed to read Compression Type");
		return EIO;
	}

	if (!TIFFGetField(tiff, TIFFTAG_BITSPERSAMPLE,
	                  &header->bits_per_sample)) {
		GP_DEBUG(1, "Failed to read Bits Per Sample");
		return EIO;
	}

	GP_DEBUG(1, "TIFF image %ux%u Compression: %s, Bits Per Sample: %u",
	         header->w, header->h,
	         compression_name(header->compress), header->bits_per_sample);

	/* If set tiff is saved in tiles */
	if (TIFFGetField(tiff, TIFFTAG_TILEWIDTH, &header->tile_w) &&
	    TIFFGetField(tiff, TIFFTAG_TILELENGTH, &header->tile_h)) {
		GP_DEBUG(1, "TIFF is tiled in %ux%u",
		         header->tile_w, header->tile_h);
		header->rows_per_strip = 0;
		return 0;
	}

	if (!TIFFGetField(tiff, TIFFTAG_ROWSPERSTRIP, &header->rows_per_strip)) {
		GP_DEBUG(1, "TIFF not saved in tiles nor strips");
		return ENOSYS;
	}

	GP_DEBUG(1, "TIFF is saved in strips");

	return 0;
}

static GP_PixelType match_grayscale_pixel_type(TIFF *tiff,
                                               struct tiff_header *header)
{
	if (!TIFFGetField(tiff, TIFFTAG_BITSPERSAMPLE,
	                  &header->bits_per_sample)) {
		GP_DEBUG(1, "Have 1bit Bitmap");
		return GP_PIXEL_G1;
	}

	switch (header->bits_per_sample) {
	case 2:
		GP_DEBUG(1, "Have 2bit Grayscale");
		return GP_PIXEL_G2;
	case 4:
		GP_DEBUG(1, "Have 4bit Grayscale");
		return GP_PIXEL_G4;
	case 8:
		GP_DEBUG(1, "Have 8bit Grayscale");
		return GP_PIXEL_G8;
	case 16:
		GP_DEBUG(1, "Have 16bit Grayscale");
		return GP_PIXEL_G16;
	default:
		GP_DEBUG(1, "Unimplemented bits per sample %u",
		         (unsigned) header->bits_per_sample);
		return GP_PIXEL_UNKNOWN;
	}
}

static GP_PixelType match_rgb_pixel_type(TIFF *tiff, struct tiff_header *header)
{
	uint16_t samples;

	if (!TIFFGetField(tiff, TIFFTAG_SAMPLESPERPIXEL, &samples)) {
		GP_DEBUG(1, "Failed to get Samples Per Pixel");
		return EINVAL;
	}

	GP_DEBUG(1, "Have %u samples per pixel", (unsigned) samples);

	uint16_t bps = header->bits_per_sample;

	/* Mach RGB pixel type with given pixel sizes */
	if (samples == 3)
		return GP_PixelRGBLookup(bps, 0, bps, bps,
		                         bps, 2*bps, 0, 0, 3*bps);

	GP_DEBUG(1, "Unsupported");
	return GP_PIXEL_UNKNOWN;
}

static GP_PixelType match_pixel_type(TIFF *tiff, struct tiff_header *header)
{
	if (!TIFFGetField(tiff, TIFFTAG_PHOTOMETRIC, &header->photometric))
		return GP_PIXEL_UNKNOWN;

	switch (header->photometric) {
	/* 1-bit or 4, 8-bit grayscale */
	case 0:
	case 1:
		return match_grayscale_pixel_type(tiff, header);
	break;
	/* RGB */
	case 2:
		return match_rgb_pixel_type(tiff, header);
	break;
	/* Palette */
	case 3:
		/* The palete is RGB161616 map it to BGR888 for now */
		return GP_PIXEL_RGB888;
	break;
	default:
		GP_DEBUG(1, "Unimplemented photometric interpretation %u",
		         (unsigned) header->photometric);
		return GP_PIXEL_UNKNOWN;
	}
}

uint16_t get_idx(uint8_t *row, uint32_t x, uint16_t bps)
{
	switch (bps) {
	case 1:
		return !!(row[x/8] & (1<<(7 - x%8)));
	case 2:
		return (row[x/4] >> (2*(3 - x%4))) & 0x03;
	case 4:
		return (row[x/2] >> (4*(!(x%2)))) & 0x0f;
	case 8:
		return row[x];
	case 16:
		return ((uint16_t*)row)[x];
	}

	GP_DEBUG(1, "Unsupported bits per sample %u", (unsigned) bps);
	return 0;
}

int tiff_read_palette(TIFF *tiff, GP_Context *res, struct tiff_header *header,
                      GP_ProgressCallback *callback)
{
	if (TIFFIsTiled(tiff)) {
		//TODO
		return ENOSYS;
	}

	if (header->bits_per_sample > 48) {
		GP_DEBUG(1, "Bits per sample too big %u",
		         (unsigned)header->bits_per_sample);
		return EINVAL;
	}

	unsigned int palette_size = (1<<header->bits_per_sample);
	uint16_t *palette_r, *palette_g, *palette_b;
	uint32_t x, y, scanline_size;

	GP_DEBUG(1, "Pallete size %u", palette_size);

	if (!TIFFGetField(tiff, TIFFTAG_COLORMAP, &palette_r, &palette_g, &palette_b)) {
		GP_DEBUG(1, "Failed to read palette");
		return EIO;
	}

	scanline_size = TIFFScanlineSize(tiff);

	GP_DEBUG(1, "Scanline size %u", (unsigned) scanline_size);

	uint8_t buf[scanline_size];

	/* Read image strips scanline by scanline */
	for (y = 0; y < header->h; y++) {
		if (TIFFReadScanline(tiff, buf, y, 0) != 1) {
			//TODO: Make use of TIFF ERROR
			GP_DEBUG(1, "Error reading scanline");
			return EIO;
		}

		for (x = 0; x < header->w; x++) {
			uint16_t i = get_idx(buf, x, header->bits_per_sample);

			if (i >= palette_size) {
				GP_WARN("Invalid palette index %u",
				         (unsigned) i);
				i = 0;
			}

			GP_Pixel p = GP_Pixel_CREATE_RGB888(palette_r[i]>>8,
					                    palette_g[i]>>8,
			                                    palette_b[i]>>8);

			GP_PutPixel_Raw_24BPP(res, x, y, p);
		}

		if (GP_ProgressCallbackReport(callback, y, res->h, res->w)) {
			GP_DEBUG(1, "Operation aborted");
			return ECANCELED;
		}
	}

	GP_ProgressCallbackDone(callback);
	return 0;
}

//Temporary, the bitendians strikes again
#include "core/GP_BitSwap.h"

/*
 * Direct read -> data in image are in right format.
 */
int tiff_read(TIFF *tiff, GP_Context *res, struct tiff_header *header,
              GP_ProgressCallback *callback)
{
	uint32_t y;
	uint16_t planar_config, samples, s;

	GP_DEBUG(1, "Reading tiff data");

	if (TIFFIsTiled(tiff)) {
		//TODO
		return ENOSYS;
	}

	//ASSERT ScanlineSize == w!

	/* Figure out number of planes */
	if (!TIFFGetField(tiff, TIFFTAG_PLANARCONFIG, &planar_config))
		planar_config = 1;

	switch (planar_config) {
	case 1:
		GP_DEBUG(1, "Planar config = 1, all samples are in one plane");
		samples = 1;
	break;
	case 2:
		if (!TIFFGetField(tiff, TIFFTAG_SAMPLESPERPIXEL, &samples)) {
			GP_DEBUG(1, "Planar config = 2, samples per pixel undefined");
			return EINVAL;
		}
		GP_DEBUG(1, "Have %u samples per pixel", (unsigned)samples);
	break;
	default:
		GP_DEBUG(1, "Unimplemented planar config = %u",
		         (unsigned)planar_config);
		return EINVAL;
	}

	/* Read image strips scanline by scanline */
	for (y = 0; y < header->h; y++) {
		uint8_t *addr = GP_PIXEL_ADDR(res, 0, y);

		//TODO: Does not work with RowsPerStrip > 1 -> needs StripOrientedIO
		for (s = 0; s < samples; s++) {
			if (TIFFReadScanline(tiff, addr, y, s) != 1) {
				//TODO: Make use of TIFF ERROR
				GP_DEBUG(1, "Error reading scanline");
				return EIO;
			}

			//Temporary, till bitendians are fixed
			switch (res->pixel_type) {
			case GP_PIXEL_G1:
				GP_BitSwapRow_B1(addr, res->bytes_per_row);
			break;
			case GP_PIXEL_G2:
				GP_BitSwapRow_B2(addr, res->bytes_per_row);
			break;
			case GP_PIXEL_G4:
				GP_BitSwapRow_B4(addr, res->bytes_per_row);
			break;
			default:
			break;
			}
		}

		if (GP_ProgressCallbackReport(callback, y, res->h, res->w)) {
			GP_DEBUG(1, "Operation aborted");
			return ECANCELED;
		}
	}

	GP_ProgressCallbackDone(callback);
	return 0;
}

GP_Context *GP_ReadTIFF(void *t, GP_ProgressCallback *callback)
{
	struct tiff_header header;
	GP_Context *res = NULL;
	GP_PixelType pixel_type;
	int err;

	if ((err = read_header(t, &header)))
		goto err1;

	pixel_type = match_pixel_type(t, &header);

	if (pixel_type == GP_PIXEL_UNKNOWN) {
		err = ENOSYS;
		goto err1;
	}

	res = GP_ContextAlloc(header.w, header.h, pixel_type);

	if (res == NULL) {
		err = errno;
		GP_DEBUG(1, "Malloc failed");
		goto err1;
	}

	switch (header.photometric) {
	/* Palette */
	case 3:
		err = tiff_read_palette(t, res, &header, callback);
	break;
	default:
		err = tiff_read(t, res, &header, callback);
	}

	if (err)
		goto err1;

	return res;
err1:
	GP_ContextFree(res);
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

static int save_grayscale(TIFF *tiff, const GP_Context *src,
                          GP_ProgressCallback *callback)
{
	uint32_t x, y;
	uint8_t buf[src->bytes_per_row];
	tsize_t ret;

	TIFFSetField(tiff, TIFFTAG_BITSPERSAMPLE, src->bpp);
	TIFFSetField(tiff, TIFFTAG_SAMPLESPERPIXEL, 1);
	TIFFSetField(tiff, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_MINISBLACK);
	TIFFSetField(tiff, TIFFTAG_FILLORDER, FILLORDER_MSB2LSB);

	for (y = 0; y < src->h; y++) {
		uint8_t *addr = GP_PIXEL_ADDR(src, 0, y);

		if (src->bpp < 8 && !src->bit_endian) {
			for (x = 0; x < src->bytes_per_row; x++) {
				switch (src->pixel_type) {
				case GP_PIXEL_G1:
					buf[x] = GP_BIT_SWAP_B1(addr[x]);
				break;
				case GP_PIXEL_G2:
					buf[x] = GP_BIT_SWAP_B2(addr[x]);
				break;
				case GP_PIXEL_G4:
					buf[x] = GP_BIT_SWAP_B4(addr[x]);
				break;
				default:
					GP_BUG("Uh, oh, do we need swap?");
				}
			}
			addr = buf;
		}

		ret = TIFFWriteEncodedStrip(tiff, y, addr, src->bytes_per_row);

		if (ret == -1) {
			//TODO TIFF ERROR
			GP_DEBUG(1, "TIFFWriteEncodedStrip failed");
			errno = EIO;
			return 1;
		}

		if (GP_ProgressCallbackReport(callback, y, src->h, src->w)) {
			GP_DEBUG(1, "Operation aborted");
			errno = ECANCELED;
			return 1;
		}
	}

	TIFFClose(tiff);

	GP_ProgressCallbackDone(callback);
	return 0;
}

static int save_rgb(TIFF *tiff, const GP_Context *src,
                    GP_ProgressCallback *callback)
{
	uint8_t buf[src->w * 3];
	uint32_t x, y;

	TIFFSetField(tiff, TIFFTAG_BITSPERSAMPLE, 8);
	TIFFSetField(tiff, TIFFTAG_SAMPLESPERPIXEL, 3);
	TIFFSetField(tiff, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_RGB);

	for (y = 0; y < src->h; y++) {
		uint8_t *addr = GP_PIXEL_ADDR(src, 0, y);

		switch (src->pixel_type) {
		case GP_PIXEL_RGB888:
			for (x = 0; x < src->bytes_per_row; x+=3) {
				buf[x + 2] = addr[x];
				buf[x + 1] = addr[x + 1];
				buf[x]     = addr[x + 2];
			}
			addr = buf;
		break;
		case GP_PIXEL_xRGB8888:
			for (x = 0; x < src->bytes_per_row; x+=4) {
				buf[3*(x/4) + 2] = addr[x];
				buf[3*(x/4) + 1] = addr[x + 1];
				buf[3*(x/4)]     = addr[x + 2];
			}
			addr = buf;
		break;
		default:
		break;
		}

		TIFFWriteEncodedStrip(tiff, y, addr, src->w * 3);

		if (GP_ProgressCallbackReport(callback, y, src->h, src->w)) {
			GP_DEBUG(1, "Operation aborted");
			errno = ECANCELED;
			return 1;
		}
	}

	TIFFClose(tiff);

	GP_ProgressCallbackDone(callback);
	return 0;
}

int GP_SaveTIFF(const GP_Context *src, const char *dst_path,
                GP_ProgressCallback *callback)
{
	TIFF *tiff;

	if (GP_PixelHasFlags(src->pixel_type, GP_PIXEL_HAS_ALPHA)) {
		GP_DEBUG(1, "Alpha channel not supported yet");
		errno = ENOSYS;
		return 1;
	}

	switch (src->pixel_type) {
	case GP_PIXEL_RGB888:
	case GP_PIXEL_BGR888:
	case GP_PIXEL_xRGB8888:
	break;
	default:
		GP_DEBUG(1, "Unsupported pixel type %s",
		         GP_PixelTypeName(src->pixel_type));
	}

	/* Open TIFF image */
	tiff = TIFFOpen(dst_path, "w");

	if (tiff == NULL) {
		GP_DEBUG(1, "Failed to open tiff '%s'", dst_path);
		return 1;
	}

	/* Set required fields */
	TIFFSetField(tiff, TIFFTAG_IMAGEWIDTH, src->w);
	TIFFSetField(tiff, TIFFTAG_IMAGELENGTH, src->h);
	TIFFSetField(tiff, TIFFTAG_ROWSPERSTRIP, 1);
	TIFFSetField(tiff, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);

	/* Save grayscale */
	if (GP_PixelHasFlags(src->pixel_type, GP_PIXEL_IS_GRAYSCALE))
		return save_grayscale(tiff, src, callback);

	switch (src->pixel_type) {
	case GP_PIXEL_RGB888:
	case GP_PIXEL_BGR888:
	case GP_PIXEL_xRGB8888:
		return save_rgb(tiff, src, callback);
	default:
	break;
	}

	GP_BUG("Should not be reached");
	return 0;
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

int GP_SaveTIFF(const GP_Context *src, const char *dst_path,
                GP_ProgressCallback *callback)
{
	errno = ENOSYS;
	return 1;
}

#endif /* HAVE_TIFF */
