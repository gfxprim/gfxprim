// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2009-2014 Cyril Hrubis <metan@ucw.cz>
 */

/*

  TIFF image support using libtiff.

 */

#include <stdint.h>
#include <inttypes.h>

#include <errno.h>
#include <string.h>

#include "../../config.h"

#include <core/gp_pixel.h>
#include <core/gp_get_put_pixel.h>
#include <core/gp_debug.h>

#include <loaders/gp_loaders.gen.h>

#define TIFF_HEADER_LITTLE "II\x2a\0"
#define TIFF_HEADER_BIG    "MM\0\x2a"

int gp_match_tiff(const void *buf)
{
	if (!memcmp(buf, TIFF_HEADER_LITTLE, 4))
		return 1;

	if (!memcmp(buf, TIFF_HEADER_BIG, 4))
		return 1;

	return 0;
}

#ifdef HAVE_TIFF

#include <tiffio.h>

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

static const char *photometric_name(uint16_t photometric)
{
	switch (photometric) {
	case PHOTOMETRIC_MINISWHITE:
		return "Min is White";
	case PHOTOMETRIC_MINISBLACK:
		return "Min is black";
	case PHOTOMETRIC_RGB:
		return "RGB";
	case PHOTOMETRIC_PALETTE:
		return "Palette";
	case PHOTOMETRIC_MASK:
		return "Mask";
	case PHOTOMETRIC_SEPARATED:
		return "Separated";
	case PHOTOMETRIC_YCBCR:
		return "YCBCR";
	case PHOTOMETRIC_CIELAB:
		return "CIELAB";
	case PHOTOMETRIC_ICCLAB:
		return "ICCLAB";
	case PHOTOMETRIC_ITULAB:
		return "ITULAB";
	case PHOTOMETRIC_LOGL:
		return "LOGL";
	case PHOTOMETRIC_LOGLUV:
		return "LOGLUV";
	default:
		return "Unknown";
	}
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

enum rec_type {
	REC_STRING,
	REC_SHORT,
	REC_FLOAT,
};

struct tag {
	int tag;
	const char *name;
	enum rec_type type;
};

static struct tag tags[] = {
	{TIFFTAG_IMAGEDESCRIPTION, "Image Description", REC_STRING},
	{TIFFTAG_MAKE, "Make", REC_STRING},
	{TIFFTAG_MODEL, "Model", REC_STRING},
	{TIFFTAG_ORIENTATION, "Orientation", REC_SHORT},
	{TIFFTAG_SAMPLESPERPIXEL, "Samples per Pixel", REC_SHORT},
	{TIFFTAG_XRESOLUTION, "X Resolution", REC_FLOAT},
	{TIFFTAG_YRESOLUTION, "Y Resolution", REC_FLOAT},
	{TIFFTAG_RESOLUTIONUNIT, "Resolution Unit", REC_SHORT},
	{TIFFTAG_SOFTWARE, "Software", REC_STRING},
	{TIFFTAG_DATETIME, "Date Time", REC_STRING},
	{TIFFTAG_ARTIST, "Artist", REC_STRING},
	{TIFFTAG_HOSTCOMPUTER, "Host Computer", REC_STRING},
	{TIFFTAG_COPYRIGHT, "Copyright", REC_STRING},
	{0, NULL, 0},
};

static void fill_metadata(TIFF *tiff, struct tiff_header *header,
                          gp_storage *storage)
{
	unsigned int i;
	int flag;
	uint16_t s;
	float f;
	gp_data_node val;

	gp_storage_add_int(storage, NULL, "Width", header->w);
	gp_storage_add_int(storage, NULL, "Height", header->h);
	gp_storage_add_string(storage, NULL, "Compression",
	                        compression_name(header->compress));
	gp_storage_add_int(storage, NULL, "Bits per Sample",
	                     header->bits_per_sample);

	for (i = 0; tags[i].name; i++) {

		val.id = tags[i].name;

		switch (tags[i].type) {
		case REC_STRING:
			val.type = GP_DATA_STRING;
			flag = TIFFGetField(tiff, tags[i].tag, &(val.value.str));
		break;
		case REC_SHORT:
			val.type = GP_DATA_INT;
			flag = TIFFGetField(tiff, tags[i].tag, &s);
			val.value.i = s;
		break;
		case REC_FLOAT:
			val.type = GP_DATA_DOUBLE;
			flag = TIFFGetField(tiff, tags[i].tag, &f);
			val.value.d = f;
		break;
		default:
			GP_WARN("Unhandled type %i", tags[i].type);
		}

		if (flag) {
			gp_storage_add(storage, NULL, &val);
			flag = 0;
		}
	}
}

static gp_pixel_type match_grayscale_pixel_type(TIFF *tiff,
                                               struct tiff_header *header)
{
	if (!TIFFGetField(tiff, TIFFTAG_BITSPERSAMPLE,
	                  &header->bits_per_sample)) {
		GP_DEBUG(1, "Have 1bit Bitmap");
		return GP_PIXEL_G1;
	}

	switch (header->bits_per_sample) {
	case 1:
		GP_DEBUG(1, "Have 1bit Bitmap");
		return GP_PIXEL_G1;
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

static gp_pixel_type match_rgb_pixel_type(TIFF *tiff, struct tiff_header *header)
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
		return gp_pixel_rgb_lookup(bps, 0, bps, bps, bps, 2 * bps, 0,
					   0, 3 * bps);

	GP_DEBUG(1, "Unsupported");
	return GP_PIXEL_UNKNOWN;
}

static gp_pixel_type match_pixel_type(TIFF *tiff, struct tiff_header *header)
{
	if (!TIFFGetField(tiff, TIFFTAG_PHOTOMETRIC, &header->photometric))
		return GP_PIXEL_UNKNOWN;

	GP_DEBUG(1, "Have photometric %s",
	         photometric_name(header->photometric));

	switch (header->photometric) {
	/* 1-bit or 4, 8-bit grayscale */
	case PHOTOMETRIC_MINISWHITE:
	case PHOTOMETRIC_MINISBLACK:
		return match_grayscale_pixel_type(tiff, header);
	case PHOTOMETRIC_RGB:
		return match_rgb_pixel_type(tiff, header);
	/* The palete is RGB161616 map it to BGR888 for now */
	case PHOTOMETRIC_PALETTE:
		return GP_PIXEL_RGB888;
	default:
		GP_DEBUG(1, "Unimplemented photometric interpretation %u",
		         (unsigned) header->photometric);
		return GP_PIXEL_UNKNOWN;
	}
}

static uint16_t get_idx(uint8_t *row, uint32_t x, uint16_t bps)
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

static int tiff_read_palette(TIFF *tiff, gp_pixmap *res,
                             struct tiff_header *header,
                             gp_progress_cb *callback)
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

			gp_pixel p = GP_PIXEL_CREATE_RGB888(palette_r[i]>>8,
					                    palette_g[i]>>8,
			                                    palette_b[i]>>8);

			gp_putpixel_raw_24BPP(res, x, y, p);
		}

		if (gp_progress_cb_report(callback, y, res->h, res->w)) {
			GP_DEBUG(1, "Operation aborted");
			return ECANCELED;
		}
	}

	gp_progress_cb_done(callback);
	return 0;
}

//Temporary, the bitendians strikes again
#include <core/gp_bit_swap.h>

/*
 * Direct read -> data in image are in right format.
 */
static int tiff_read(TIFF *tiff, gp_pixmap *res, struct tiff_header *header,
                     gp_progress_cb *callback)
{
	uint32_t i, y;
	uint16_t planar_config, samples, s;

	GP_DEBUG(1, "Reading tiff data");

	if (TIFFIsTiled(tiff)) {
		//TODO
		return ENOSYS;
	}

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

			/* We need to negate the values when Min is White */
			if (header->photometric == PHOTOMETRIC_MINISWHITE)
				for (i = 0; i < res->bytes_per_row; i++)
					addr[i] = ~addr[i];
		}

		if (gp_progress_cb_report(callback, y, res->h, res->w)) {
			GP_DEBUG(1, "Operation aborted");
			return ECANCELED;
		}
	}

	gp_progress_cb_done(callback);
	return 0;
}

static tsize_t tiff_io_read(thandle_t io, void *buf, tsize_t size)
{
	return gp_io_read(io, buf, size);
}

static tsize_t tiff_io_write(thandle_t io, void *buf, tsize_t size)
{
	return gp_io_write(io, buf, size);
}

static toff_t tiff_io_seek(thandle_t io, toff_t offset, int whence)
{
	return gp_io_seek(io, offset, whence);
}

static int tiff_io_close(thandle_t GP_UNUSED(io))
{
	return 0;
}

static toff_t tiff_io_size(thandle_t io)
{
	return gp_io_size(io);
}

/*
static int tiff_io_map(thandle_t io, void **base, toff_t *size)
{
	GP_WARN("stub called");
	return 0;
}

static void tiff_io_unmap(thandle_t io, void *base, toff_t size)
{
	GP_WARN("stub called");
	return 0;
}
*/

int gp_read_tiff_ex(gp_io *io, gp_pixmap **img, gp_storage *storage,
                  gp_progress_cb *callback)
{
	TIFF *tiff;
	struct tiff_header header;
	gp_pixmap *res;
	gp_pixel_type pixel_type;
	int err;

	tiff = TIFFClientOpen("GFXprim IO", "r", io, tiff_io_read,
	                      tiff_io_write, tiff_io_seek, tiff_io_close,
	                      tiff_io_size, NULL, NULL);

	if (!tiff) {
		GP_DEBUG(1, "TIFFClientOpen failed");
		err = EIO;
		goto err0;
	}

	if ((err = read_header(tiff, &header)))
		goto err1;

	if (storage)
		fill_metadata(tiff, &header, storage);

	if (!img)
		return 0;

	pixel_type = match_pixel_type(tiff, &header);

	if (pixel_type == GP_PIXEL_UNKNOWN) {
		err = ENOSYS;
		goto err1;
	}

	res = gp_pixmap_alloc(header.w, header.h, pixel_type);

	if (res == NULL) {
		err = errno;
		GP_DEBUG(1, "Malloc failed");
		goto err1;
	}

	if (TIFFScanlineSize(tiff) > res->bytes_per_row) {
		GP_WARN("ScanlineSize %li > bytes_per_row %i",
		        TIFFScanlineSize(tiff), res->bytes_per_row);
		return EINVAL;
	}

	switch (header.photometric) {
	case PHOTOMETRIC_PALETTE:
		err = tiff_read_palette(tiff, res, &header, callback);
	break;
	default:
		err = tiff_read(tiff, res, &header, callback);
	}

	if (err)
		goto err2;

	TIFFClose(tiff);

	*img = res;
	return 0;
err2:
	gp_pixmap_free(res);
err1:
	TIFFClose(tiff);
err0:
	errno = err;
	return 1;
}

static int save_grayscale(TIFF *tiff, const gp_pixmap *src,
                          gp_progress_cb *callback)
{
	uint32_t y;

	TIFFSetField(tiff, TIFFTAG_BITSPERSAMPLE, gp_pixel_size(src->pixel_type));
	TIFFSetField(tiff, TIFFTAG_SAMPLESPERPIXEL, 1);
	TIFFSetField(tiff, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_MINISBLACK);
	TIFFSetField(tiff, TIFFTAG_FILLORDER, FILLORDER_MSB2LSB);

	for (y = 0; y < src->h; y++) {
		uint8_t *addr = GP_PIXEL_ADDR(src, 0, y);
		tsize_t ret;

		ret = TIFFWriteEncodedStrip(tiff, y, addr, src->bytes_per_row);

		if (ret == -1) {
			//TODO TIFF ERROR
			GP_DEBUG(1, "TIFFWriteEncodedStrip failed");
			return EIO;
		}

		if (gp_progress_cb_report(callback, y, src->h, src->w)) {
			GP_DEBUG(1, "Operation aborted");
			return ECANCELED;
		}
	}

	return 0;
}

static int save_rgb(TIFF *tiff, const gp_pixmap *src,
                    gp_progress_cb *callback)
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

		if (gp_progress_cb_report(callback, y, src->h, src->w)) {
			GP_DEBUG(1, "Operation aborted");
			return ECANCELED;
		}
	}

	return 0;
}

static gp_pixel_type save_ptypes[] = {
	GP_PIXEL_BGR888,
	GP_PIXEL_RGB888,
	GP_PIXEL_xRGB8888,
	GP_PIXEL_G1,
	GP_PIXEL_G2,
	GP_PIXEL_G4,
	GP_PIXEL_G8,
	GP_PIXEL_UNKNOWN,
};

int gp_write_tiff(const gp_pixmap *src, gp_io *io,
                 gp_progress_cb *callback)
{
	TIFF *tiff;
	int err = 0;

	GP_DEBUG(1, "Writing TIFF to I/O (%p)", io);

	if (gp_pixel_has_flags(src->pixel_type, GP_PIXEL_HAS_ALPHA)) {
		GP_DEBUG(1, "Alpha channel not supported yet");
		errno = ENOSYS;
		return 1;
	}

	switch (src->pixel_type) {
	case GP_PIXEL_G1:
	case GP_PIXEL_G2:
	case GP_PIXEL_G4:
	case GP_PIXEL_G8:
	break;
	case GP_PIXEL_RGB888:
	case GP_PIXEL_BGR888:
	case GP_PIXEL_xRGB8888:
	break;
	default:
		GP_DEBUG(1, "Unsupported pixel type %s",
		         gp_pixel_type_name(src->pixel_type));
		errno = ENOSYS;
		return 1;
	}

	/* Open TIFF image */
	tiff = TIFFClientOpen("GFXprim IO", "w", io, tiff_io_read,
	                      tiff_io_write, tiff_io_seek, tiff_io_close,
	                      tiff_io_size, NULL, NULL);

	if (!tiff) {
		GP_DEBUG(1, "TIFFClientOpen failed");
		errno = EIO;
		return 1;
	}

	/* Set required fields */
	TIFFSetField(tiff, TIFFTAG_IMAGEWIDTH, src->w);
	TIFFSetField(tiff, TIFFTAG_IMAGELENGTH, src->h);
	TIFFSetField(tiff, TIFFTAG_ROWSPERSTRIP, 1);
	TIFFSetField(tiff, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);

	switch (src->pixel_type) {
	case GP_PIXEL_RGB888:
	case GP_PIXEL_BGR888:
	case GP_PIXEL_xRGB8888:
		err = save_rgb(tiff, src, callback);
	break;
	case GP_PIXEL_G1:
	case GP_PIXEL_G2:
	case GP_PIXEL_G4:
	case GP_PIXEL_G8:
		err = save_grayscale(tiff, src, callback);
	break;
	default:
		GP_BUG("Wrong pixel type");
	break;
	}

	if (err) {
		TIFFClose(tiff);
		errno = err;
		return 1;
	}

	TIFFClose(tiff);
	gp_progress_cb_done(callback);
	return 0;
}

#else

int gp_read_tiff_ex(gp_io GP_UNUSED(*io), gp_pixmap GP_UNUSED(**img),
                  gp_storage GP_UNUSED(*storage),
                  gp_progress_cb GP_UNUSED(*callback))
{
	errno = ENOSYS;
	return 1;
}

int gp_write_tiff(const gp_pixmap GP_UNUSED(*src), gp_io GP_UNUSED(*io),
                 gp_progress_cb GP_UNUSED(*callback))
{
	errno = ENOSYS;
	return 1;
}

#endif /* HAVE_TIFF */

const struct gp_loader gp_tiff = {
#ifdef HAVE_TIFF
	.read = gp_read_tiff_ex,
	.write = gp_write_tiff,
	.save_ptypes = save_ptypes,
#endif
	.match = gp_match_tiff,

	.fmt_name = "Tag Image File Format",
	.extensions = {"tif", "tiff", NULL},
};
