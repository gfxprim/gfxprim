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

  Photoshop PSD image loader.

  Written using documentation available freely on the internet.

 */

#include <stdint.h>
#include <inttypes.h>
#include <string.h>
#include <errno.h>

#include "core/GP_Debug.h"
#include "core/GP_Common.h"
#include "core/GP_GetPutPixel.h"
#include "GP_JPG.h"
#include "GP_PSD.h"

#define PSD_SIGNATURE "8BPS\x00\x01"
#define PSD_SIGNATURE_LEN 6

int GP_MatchPSD(const void *buf)
{
	return !memcmp(buf, PSD_SIGNATURE, PSD_SIGNATURE_LEN);
}

enum psd_img_res_id {
	PSD_RESOLUTION_INFO       = 1005,
	PSD_ALPHA_CHANNELS_NAMES  = 1006,
	PSD_DISPLAY_INFO0         = 1007,
	PSD_BACKGROUND_COLOR      = 1010,
	PSD_PRINT_FLAGS           = 1011,
	PSD_COLOR_HALFTONING_INFO = 1013,
	PSD_COLOR_TRANSFER_FUNC   = 1016,
	PSD_LAYER_STATE_INFO      = 1024,
	PSD_WORKING_PATH          = 1025,
	PSD_LAYERS_GROUP_INFO     = 1026,
	PSD_IPTC_NAA_REC          = 1028,
	PSD_GRID_AND_GUIDES       = 1032,
	PSD_THUMBNAIL_RES40       = 1033,
	PSD_COPYRIGHT_FLAG        = 1034,
	PSD_THUMBNAIL_RES50       = 1036,
	PSD_GLOBAL_ANGLE          = 1037,
	PSD_ICC_PROFILE           = 1039,
	PSD_ICC_UNTAGGED_PROFILE  = 1041,
	PSD_ID_SEED               = 1044,
	PSD_UNICODE_ALPHA_NAMES   = 1045,
	PSD_GLOBAL_ALTITUDE       = 1049,
	PSD_SLICES                = 1050,
	PSD_ALPHA_IDENTIFIERS     = 1053,
	PSD_URL_LIST              = 1054,
	PSD_VERSION_INFO          = 1057,
	PSD_EXIF1                 = 1058,
	PSD_EXIF3                 = 1059,
	PSD_XML_METADATA          = 1060,
	PSD_CAPTION_DIGETST       = 1061,
	PSD_PRINT_SCALE           = 1062,
	PSD_PIXEL_ASPECT_RATIO    = 1064,
	PSD_LAYER_SELECTION_IDS   = 1069,
	PSD_PRINT_INFO_CS2        = 1071,
	PSD_LAYER_GROUPS          = 1072,
	PSD_TIMELINE_INFO         = 1075,
	PSD_SHEET_DISCLOSURE      = 1076,
	PSD_DISPLAY_INFO          = 1077,
	PSD_ONION_SKINS           = 1078,
	PSD_PRINT_INFO_CS5        = 1082,
	PSD_PRINT_STYLE           = 1083,
	PSD_PRINT_FLAGS_INFO      = 10000,
};

static const char *psd_img_res_id_name(uint16_t id)
{
	switch (id) {
	case PSD_RESOLUTION_INFO:
		return "Resolution Info";
	case PSD_ALPHA_CHANNELS_NAMES:
		return "Alpha Channels Names";
	case PSD_DISPLAY_INFO0:
		return "Display Info (Obsolete)";
	case PSD_BACKGROUND_COLOR:
		return "Background Color";
	case PSD_PRINT_FLAGS:
		return "Print Flags";
	case PSD_COLOR_HALFTONING_INFO:
		return "Color Halftoning Info";
	case PSD_COLOR_TRANSFER_FUNC:
		return "Color Transfer Function";
	case PSD_LAYER_STATE_INFO:
		return "Layer State Info";
	case PSD_WORKING_PATH:
		return "Working Path";
	case PSD_LAYERS_GROUP_INFO:
		return "Layers Group Info";
	case PSD_IPTC_NAA_REC:
		return "IPTC-NAA Record";
	case PSD_GRID_AND_GUIDES:
		return "Grid and Guides";
	case PSD_THUMBNAIL_RES40:
		return "Thumbnail 4.0";
	case PSD_COPYRIGHT_FLAG:
		return "Copyright Flag";
	case PSD_THUMBNAIL_RES50:
		return "Thumbnail 5.0+";
	case PSD_GLOBAL_ANGLE:
		return "Global Angle";
	case PSD_ICC_PROFILE:
		return "ICC Profile";
	case PSD_ICC_UNTAGGED_PROFILE:
		return "ICC Untagged Profile";
	case PSD_ID_SEED:
		return "ID Seed";
	case PSD_UNICODE_ALPHA_NAMES:
		return "Unicode Alpha Names";
	case PSD_GLOBAL_ALTITUDE:
		return "Global Altitude";
	case PSD_SLICES:
		return "Slices";
	case PSD_ALPHA_IDENTIFIERS:
		return "Alpha Identifiers";
	case PSD_URL_LIST:
		return "URL List";
	case PSD_VERSION_INFO:
		return "Version Info";
	case PSD_EXIF1:
		return "Exif data 1";
	case PSD_EXIF3:
		return "Exif data 3";
	case PSD_XML_METADATA:
		return "XML Metadata";
	case PSD_CAPTION_DIGETST:
		return "Caption Digest";
	case PSD_PRINT_SCALE:
		return "Print Scale";
	case PSD_PIXEL_ASPECT_RATIO:
		return "Pixel Aspect Ratio";
	case PSD_LAYER_SELECTION_IDS:
		return "Selectin IDs";
	case PSD_PRINT_INFO_CS2:
		return "Print Info CS2";
	case PSD_LAYER_GROUPS:
		return "Layer Groups";
	case PSD_TIMELINE_INFO:
		return "Timeline Info";
	case PSD_SHEET_DISCLOSURE:
		return "Sheet Disclosure";
	case PSD_DISPLAY_INFO:
		return "Display Info";
	case PSD_ONION_SKINS:
		return "Onion Skins";
	case PSD_PRINT_INFO_CS5:
		return "Print Info CS5";
	case PSD_PRINT_STYLE:
		return "Print Scale";
	case 4000 ... 4999:
		return "Plugin Resource";
	case PSD_PRINT_FLAGS_INFO:
		return "Print Flags Information";
	default:
		return "Unknown";
	}
}

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

#define THUMBNAIL50_HSIZE 28

static GP_Context *psd_thumbnail50(GP_IO *io, size_t size,
                                   GP_ProgressCallback *callback)
{
	uint32_t fmt, w, h;
	uint16_t bpp, nr_planes;
	GP_Context *res;
	int err;

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

	GP_DEBUG(3, "%"PRIu32"x%"PRIu32" format=%s (%"PRIu16") bpp=%"PRIu16
	         " nr_planes=%"PRIu16, w, h, thumbnail_fmt_name(fmt), fmt,
		 bpp, nr_planes);

	if (fmt != PSD_JPG_RGB) {
		GP_DEBUG(1, "Unsupported thumbnail format");
		return NULL;
	}

	if (size < THUMBNAIL50_HSIZE) {
		GP_WARN("Negative thumbnail resource size");
		errno = EINVAL;
		return NULL;
	}

	GP_IO *sub_io = GP_IOSubIO(io, size - THUMBNAIL50_HSIZE);

	if (!sub_io)
		return NULL;

	res = GP_ReadJPG(sub_io, callback);
	err = errno;
	GP_IOClose(sub_io);
	errno = err;

	return res;
}

static unsigned int read_unicode_string(GP_IO *io, char *str,
                                        unsigned int size)
{
	uint8_t buf[size * 2];
	unsigned int i;

	if (GP_IOFill(io, buf, size * 2)) {
		GP_DEBUG(1, "Failed to read unicode string");
		return 0;
	}

	for (i = 0; i < size; i++) {
		if (buf[i * 2])
			str[i] = '?';
		else
			str[i] = buf[i * 2 + 1];
	}

	str[size] = 0;

	return size * 2;
}

static void psd_read_version_info(GP_IO *io)
{
	unsigned int size = 13;
	uint32_t version, str_size;

	uint16_t version_data[] = {
		GP_IO_B4, /* Version */
		GP_IO_I1, /* hasRealMergedData ??? */
		GP_IO_B4, /* Unicode version string size */
		GP_IO_END
	};

	if (GP_IOReadF(io, version_data, &version, &str_size) != 3) {
		GP_DEBUG(1, "Failed to read version header");
		return;
	}

	//TODO: Check str size is small
	char str[str_size + 1];

	size += read_unicode_string(io, str, str_size);

	if (GP_IOReadB4(io, &str_size)) {
		GP_DEBUG(1, "Failed to read string size");
		return;
	}

	char reader[str_size + 1];

	size += read_unicode_string(io, reader, str_size);

	GP_DEBUG(3, "Version %"PRIu32" writer='%s' reader='%s'",
	         version, str, reader);
}

static unsigned int psd_next_img_res_block(GP_IO *io, GP_Context **res,
                                           GP_ProgressCallback *callback)
{
	uint16_t res_id;
	uint32_t res_size, seek_size;
	off_t prev, after;

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

	GP_DEBUG(2, "Image resource id=%-5"PRIu16" size=%-8"PRIu32" (%s)",
	         res_id, res_size, psd_img_res_id_name(res_id));

	seek_size = res_size = GP_ALIGN2(res_size);

	switch (res_id) {
	case PSD_THUMBNAIL_RES40:
		GP_DEBUG(1, "Unsupported thumbnail version 4.0");
	break;
	case PSD_THUMBNAIL_RES50:
		prev = GP_IOTell(io);
		*res = psd_thumbnail50(io, res_size, callback);
		after = GP_IOTell(io);
		seek_size -= (after - prev);
	break;
	case PSD_VERSION_INFO:
		prev = GP_IOTell(io);
		psd_read_version_info(io);
		after = GP_IOTell(io);
		seek_size -= (after - prev);
	break;
	}

	if (GP_IOSeek(io, seek_size, GP_IO_SEEK_CUR) == (off_t)-1) {
		GP_DEBUG(1, "Failed skip image resource");
		return 0;
	}

	return res_size + 12;
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

enum compress_method {
	PSD_COMPRESS_RAW    = 0,
	PSD_COMPRESS_RLE    = 1,
	PSD_COMPRESS_ZIP_NP = 2,
	PSD_COMPRESS_ZIP_WP = 3,
};

struct psd_header {
	uint32_t w;
	uint32_t h;
	uint16_t depth;
	uint16_t channels;
	uint16_t color_mode;
};

static const char *compress_method_type(uint16_t compress)
{
	switch (compress) {
	case PSD_COMPRESS_RAW:
		return "Raw";
	case PSD_COMPRESS_RLE:
		return "RLE";
	case PSD_COMPRESS_ZIP_NP:
		return "ZIP (no prediction)";
	case PSD_COMPRESS_ZIP_WP:
		return "ZIP (with prediction)";
	default:
		return "Unknown";
	}
}

/*
 * PSD RLE I/O Stream
 */
struct rle {
	/* RLE State */
	int8_t op;
	unsigned int size;
	uint8_t val;

	/* Source I/O stream */
	GP_IO *io;

	/* Read buffer */
	ssize_t buf_fill;
	ssize_t buf_pos;
	uint8_t buf[1024];
};

static int rle_getc(struct rle *rle)
{
	if (rle->buf_pos < rle->buf_fill)
		return rle->buf[rle->buf_pos++];

	rle->buf_fill = GP_IORead(rle->io, rle->buf, sizeof(rle->buf));

	if (rle->buf_fill <= 0)
		return -1;

	rle->buf_pos = 1;
	return rle->buf[0];
}

static int rle_readb(struct rle *rle, void *buf, size_t size)
{
	ssize_t bytes = rle->buf_fill - rle->buf_pos;
	size_t s = GP_MIN(size, (size_t)bytes);

	memcpy(buf, rle->buf + rle->buf_pos, s);

	rle->buf_pos += s;

	if (s < size)
		return GP_IOFill(rle->io, buf + s, size - s);

	return 0;
}

static ssize_t rle_read(GP_IO *self, void *buf, size_t size)
{
	struct rle *rle = GP_IO_PRIV(self);
	size_t read = 0, s;
	uint8_t *bbuf = buf;

	while (read < size) {
		switch (rle->op) {
		case -128:
			rle->op = rle_getc(rle);
		break;
		case 0 ... 127:
			rle->size = rle->op + 1;
			s = GP_MIN(rle->size, read - size);
			rle_readb(rle, bbuf + read, s);
			rle->size -= s;
			read += s;

//			while (rle->size-- && read < size)
//				bbuf[read++] = rle_getc(rle);

			if (rle->size <= 0)
				rle->op = -128;
		break;
		case -127 ... -1:
			rle->size = 1 - rle->op;
			rle->val = rle_getc(rle);

			s = GP_MIN(rle->size, size - read);
			memset(bbuf + read, rle->val, s);
			rle->size -= s;
			read += s;

//			while (rle->size-- && read < size)
//				bbuf[read++] = rle->val;

			if (rle->size <= 0)
				rle->op = -128;
		break;
		}
	}

	return read;
}

static int rle_close(GP_IO *self)
{
	free(self);
	return 0;
}

static GP_IO *rle(GP_IO *io)
{
	GP_IO *rle = malloc(sizeof(GP_IO) + sizeof(struct rle));

	if (!rle)
		return NULL;

	struct rle *priv = GP_IO_PRIV(rle);

	priv->op = -128;
	priv->buf_fill = 0;
	priv->buf_pos = 0;
	priv->io = io;

	rle->Read = rle_read;
	rle->Write = NULL;
	rle->Seek = NULL;
	rle->Close = rle_close;

	return rle;
}

/*
 * Data are in planar mode RRRRR... GGGGG... BBBBB... etc.
 */
static int psd_load_rle_rgb(GP_IO *rle_io, GP_Context *res,
                            GP_ProgressCallback *callback)
{
	unsigned int x, y, c, p;
	unsigned int chans = res->pixel_type == GP_PIXEL_RGB888 ? 3 : 4;
	uint8_t b[res->w], *bp;

	for (c = 0; c < chans; c++) {
		switch (c) {
		case 0:
			p = 2;
		break;
		case 2:
			p = 0;
		break;
		default:
			p = c;
		}

		for (y = 0; y < res->h; y++) {
			bp = GP_PIXEL_ADDR(res, 0, y);
			GP_IORead(rle_io, b, sizeof(b));

			for (x = 0; x < res->w; x++)
				bp[x * chans + p] = b[x];

			if (GP_ProgressCallbackReport(callback, res->h * c + y,
			                              res->h * chans, res->w)) {
				GP_DEBUG(1, "Operation aborted");
				return ECANCELED;
			}
		}
	}

	GP_ProgressCallbackDone(callback);

	return 0;
}

static int psd_load_rle_cmyk(GP_IO *rle_io, GP_Context *res,
                             GP_ProgressCallback *callback)
{
	unsigned int x, y, c;
	uint8_t b[res->w], *bp;

	for (c = 0; c < 4; c++) {
		for (y = 0; y < res->h; y++) {
			bp = GP_PIXEL_ADDR(res, 0, y);
			GP_IORead(rle_io, b, sizeof(b));

			for (x = 0; x < res->w; x++)
				bp[x * 4 + c] = 255 - b[x];

			if (GP_ProgressCallbackReport(callback, res->h * c + y,
			                              4 * res->h, res->w)) {
				GP_DEBUG(1, "Operation aborted");
				return ECANCELED;
			}
		}
	}

	GP_ProgressCallbackDone(callback);

	return 0;
}

static int psd_combined_image(GP_IO *io, struct psd_header *header,
                              GP_Context **res, GP_ProgressCallback *callback)
{
	uint16_t compress;
	GP_PixelType pixel_type = GP_PIXEL_UNKNOWN;
	int ret;

	if (GP_IOReadB2(io, &compress)) {
		GP_DEBUG(1, "Failed to read Combined Image compression");
		return errno;
	}

	GP_DEBUG(1, "Combined image compression %s (%"PRIu16")",
	         compress_method_type(compress), compress);

	if (compress != PSD_COMPRESS_RLE) {
		GP_DEBUG(1, "Unsupported compression");
		return ENOSYS;
	}

	if (header->color_mode == PSD_RGB) {
		switch (header->channels) {
		case 3:
			switch (header->depth) {
			case 8:
				pixel_type = GP_PIXEL_RGB888;
			break;
			}
		break;
		case 4:
			switch (header->depth) {
			case 8:
				pixel_type = GP_PIXEL_RGBA8888;
			break;
			}
		break;
		}
	}

	if (header->color_mode == PSD_CMYK && header->channels == 4) {
		switch (header->channels) {
		case 4:
			pixel_type = GP_PIXEL_CMYK8888;
		break;
		}
	}

	if (pixel_type == GP_PIXEL_UNKNOWN) {
		GP_DEBUG(1, "Unsupported color_mode/channels/bpp combination");
		return ENOSYS;
	}

	/*
	 * Skip line byte counts
	 *
	 * Two bytes per channel per row
	 */
	if (GP_IOSeek(io, 2 * header->channels * header->h, GP_IO_SEEK_CUR) == (off_t)-1) {
		GP_DEBUG(1, "Failed to skip Line Bytes Counts");
		return errno;
	}

	*res = GP_ContextAlloc(header->w, header->h, pixel_type);

	if (!*res)
		return ENOMEM;

	GP_IO *rle_io = rle(io);
	if (!rle_io)
		return ENOMEM;

	if (pixel_type == GP_PIXEL_CMYK8888)
		ret = psd_load_rle_cmyk(rle_io, *res, callback);
	else
		ret = psd_load_rle_rgb(rle_io, *res, callback);

	if (ret)
		GP_ContextFree(*res);

	GP_IOClose(rle_io);
	return ret;
}

GP_Context *GP_ReadPSD(GP_IO *io, GP_ProgressCallback *callback)
{
	int err;
	uint32_t len, size, read_size = 0;
	struct psd_header header;
	GP_Context *thumbnail = NULL;

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

	if (GP_IOReadF(io, psd_header, &header.channels, &header.h, &header.w,
	               &header.depth, &header.color_mode, &len) != 13) {
		GP_DEBUG(1, "Failed to read file header");
		err = errno;
		goto err;
	}

	GP_DEBUG(1, "Have PSD %"PRIu32"x%"PRIu32" channels=%"PRIu16","
	         " bpp=%"PRIu16" color_mode=%s (%"PRIu16") "
	         " color_mode_data_len=%"PRIu32, header.w, header.h,
	         header.channels, header.depth,
	         psd_color_mode_name(header.color_mode), header.color_mode, len);

	switch (header.color_mode) {
	case PSD_INDEXED:
	case PSD_DUOTONE:
	break;
	default:
		if (len) {
			GP_WARN("Color mode_mode_data_len != 0 (is %"PRIu32")"
			        "for %s (%"PRIu16")", len,
			        psd_color_mode_name(header.color_mode),
		                header.color_mode);
		}
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

	GP_DEBUG(1, "Image Resource Section length is %u", len);

	do {
		size = psd_next_img_res_block(io, &thumbnail, NULL);

		if (size == 0)
			return thumbnail;

		read_size += size;
	} while (read_size < len);

	/* Skip Layer and Mask information */
	if (GP_IOReadB4(io, &size)) {
		GP_DEBUG(1, "Failed to read Layer and Mask Section size");
		err = errno;
		goto err;
	}

	if (GP_IOSeek(io, size, GP_IO_SEEK_CUR) == (off_t)-1) {
		GP_DEBUG(1, "Failed to seek to Image Data Section");
		err = errno;
		goto err;
	}

	GP_Context *combined = NULL;
	err = psd_combined_image(io, &header, &combined, callback);

	if (!err) {
		GP_ContextFree(thumbnail);
		return combined;
	}

	if (err == ECANCELED)
		goto err;

	if (thumbnail)
		return thumbnail;

	errno = ENOSYS;
	return NULL;
err:
	GP_ContextFree(thumbnail);
	errno = err;
	return NULL;
}

GP_Context *GP_LoadPSD(const char *src_path, GP_ProgressCallback *callback)
{
	return GP_LoaderLoadImage(&GP_PSD, src_path, callback);
}

struct GP_Loader GP_PSD = {
	.Read = GP_ReadPSD,
	.Match = GP_MatchPSD,

	.fmt_name = "Adobe Photoshop Image",
	.extensions = {"psd", NULL},
};
