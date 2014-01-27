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

  PCX image support.

 */

#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>

#include "core/GP_Debug.h"
#include "core/GP_GetPutPixel.h"

#include "GP_PCX.h"

/* Creator ZSoft: 0x0a
 * Version:       0x00, 0x02, 0x03, 0x04, 0x05
 * RLE:           0x01
 * BPP:           0x01, 0x02, 0x04, 0x08
 */
int GP_MatchPCX(const void *buf)
{
	const uint8_t *b = buf;

	if (b[0] != 0x0a)
		return 0;

	/* version */
	switch (b[1]) {
	case 0:
	case 2:
	case 3:
	case 4:
	case 5:
	break;
	default:
		return 0;
	}

	if (b[2] != 0x01)
		return 0;

	/* bpp 1, 2, 4 or 8 */
	switch (b[3]) {
	case 1:
	case 2:
	case 4:
	case 8:
		return 1;
	default:
		return 0;
	}
}

struct pcx_header {
	uint8_t ver;
	uint8_t bpp;
	uint16_t xs, ys, xe, ye;
	uint16_t hres, vres;
	uint8_t nplanes;
	uint16_t pal_info;
	uint16_t bytes_per_line;
	/* 16 RGB tripplets palette */
	uint8_t palette[48];
};

static int get_byte(GP_IO *io)
{
	uint8_t buf;

	if (GP_IORead(io, &buf, 1) != 1)
		return -1;

	return buf;
}

static int readline(GP_IO *io, uint8_t *buf, unsigned int size, unsigned int padd)
{
	int b, val = 0, cnt = 0;
	unsigned int read = 0;

	for (;;) {
		while (cnt > 0 && read < size) {
			buf[read++] = val;
			cnt--;
		}

		if (read >= size) {
			goto end;
		}

		b = get_byte(io);

		if (b == -1) {
			GP_WARN("End of file reached unexpectedly");
			return 0;
		}

		if ((b & 0xc0) == 0xc0) {
			cnt = b & 0x3f;
			val = get_byte(io);
		} else {
			cnt = 1;
			val = b;
		}
	}

end:
	/*
	 * Data may be padded, read the excess bytes
	 */
	while (padd--) {
		if (cnt) {
			cnt--;
		} else {
			b = get_byte(io);
			if ((b & 0xc0) == 0xc0) {
				cnt = b & 0x3f;
				get_byte(io);
			}
		}
	}

	if (cnt)
		GP_WARN("Nonzero repeat count at the line end (%u)", cnt);

	return 0;
}

#include "core/GP_BitSwap.h"

static int read_g1(GP_IO *io, struct pcx_header *header,
                   GP_Context *res, GP_ProgressCallback *callback)
{
	uint32_t y;
	int padd = (int)header->bytes_per_line - (int)res->bytes_per_row;

	if (padd < 0) {
		GP_WARN("Invalid number of bytes per line");
		return EINVAL;
	}

	for (y = 0; y < res->h; y++) {
		uint8_t *addr = GP_PIXEL_ADDR(res, 0, y);

		readline(io, addr, res->bytes_per_row, padd);

		//TODO: FIX Endians
		GP_BitSwapRow_B1(addr, res->bytes_per_row);

		if (GP_ProgressCallbackReport(callback, y, res->h, res->w)) {
			GP_DEBUG(1, "Operation aborted");
			return ECANCELED;
		}
	}

	return 0;
}

static int read_rgb888(GP_IO *io, struct pcx_header *header,
                       GP_Context *res, GP_ProgressCallback *callback)
{
	uint32_t x, y;
	unsigned int bpr = header->bytes_per_line;
	uint8_t b[3 * bpr];

	for (y = 0; y < res->h; y++) {
		readline(io, b, sizeof(b), 0);

		for (x = 0; x < res->w; x++) {
			GP_Pixel pix = GP_Pixel_CREATE_RGB888(b[x],
			                                      b[x+bpr],
			                                      b[x+2*bpr]);
			GP_PutPixel_Raw_24BPP(res, x, y, pix);
		}

		if (GP_ProgressCallbackReport(callback, y, res->h, res->w)) {
			GP_DEBUG(1, "Operation aborted");
			return ECANCELED;
		}
	}

	return 0;
}

static int read_16_palette(GP_IO *io, struct pcx_header *header,
                           GP_Context *res, GP_ProgressCallback *callback)
{
	uint32_t x, y;
	unsigned int i;
	uint8_t b[header->bytes_per_line];
	GP_Pixel palette[16];
	uint8_t idx=0, mask, mod;

	for (i = 0; i < 16; i++) {
		palette[i] = (GP_Pixel)header->palette[3*i] << 16;
		palette[i] |= (GP_Pixel)header->palette[3*i+1] << 8;
		palette[i] |= header->palette[3*i+2];
	}

	switch (header->bpp) {
	case 2:
		mask = 0x30;
		mod = 4;
	break;
	case 4:
		mask = 0xf0;
		mod = 2;
	break;
	default:
		GP_BUG("Invalid 16 color palette bpp %u", header->bpp);
		return EINVAL;
	}

	for (y = 0; y < res->h; y++) {
		readline(io, b, sizeof(b), 0);

		i = 0;

		for (x = 0; x < res->w; x++) {

			if (!(x % mod))
				idx = b[i++];

			GP_PutPixel_Raw_24BPP(res, x, y, palette[(idx & mask) >> (8 - header->bpp)]);
			idx <<= header->bpp;
		}

		if (GP_ProgressCallbackReport(callback, y, res->h, res->w)) {
			GP_DEBUG(1, "Operation aborted");
			return ECANCELED;
		}
	}

	return 0;
}

static int read_256_palette(GP_IO *io, struct pcx_header *header,
                            GP_Context *res, GP_ProgressCallback *callback)
{
	uint32_t x, y;
	unsigned int i;
	GP_Pixel palette[256];
	uint8_t b[header->bytes_per_line];

	if (GP_IOSeek(io, -769, GP_IO_SEEK_END) == (off_t)-1) {
		GP_DEBUG(1, "Failed to seek to palette: %s", strerror(errno));
		return EIO;
	}

	if (get_byte(io) != 0x0c) {
		GP_DEBUG(1, "Wrong palette marker");
		return EIO;
	}

	for (i = 0; i < 256; i++) {
		palette[i] = get_byte(io) << 16;
		palette[i] |= get_byte(io) << 8;
		palette[i] |= get_byte(io);
	}

	if (GP_IOSeek(io, 128, GP_IO_SEEK_SET) == (off_t)-1) {
		GP_DEBUG(1, "Failed to seek to image data: %s",
		         strerror(errno));
		return EIO;
	}

	for (y = 0; y < res->h; y++) {
		readline(io, b, sizeof(b), 0);

		for (x = 0; x < res->w; x++)
			GP_PutPixel_Raw_24BPP(res, x, y, palette[b[x]]);

		if (GP_ProgressCallbackReport(callback, y, res->h, res->w)) {
			GP_DEBUG(1, "Operation aborted");
			return ECANCELED;
		}
	}

	return 0;
}

static GP_PixelType match_pixel_type(struct pcx_header *header)
{
	switch (header->nplanes) {
	case 1:
		switch (header->bpp) {
		/* 1 bit grayscale */
		case 1:
			return GP_PIXEL_G1;
		/* 16 color palette */
		case 2:
		case 4:
		/* 256 color palette */
		case 8:
			return GP_PIXEL_RGB888;
		}
	break;
	/* raw RGB */
	case 3:
		if (header->bpp == 8)
			return GP_PIXEL_RGB888;
	break;
	}

	return GP_PIXEL_UNKNOWN;
}

static int read_image(GP_IO *io, struct pcx_header *header,
                      GP_Context *res, GP_ProgressCallback *callback)
{
	switch (header->nplanes) {
	case 1:
		switch (header->bpp) {
		case 1:
			return read_g1(io, header, res, callback);
		case 2:
		case 4:
			return read_16_palette(io, header, res, callback);
		case 8:
			return read_256_palette(io, header, res, callback);
		}
	break;
	case 3:
		if (header->bpp == 8)
			return read_rgb888(io, header, res, callback);
	break;
	default:
	break;
	}

	GP_BUG("Have pixel type %s but cannot load image data",
	       GP_PixelTypeName(res->pixel_type));
	return ENOSYS;
}

GP_Context *GP_ReadPCX(GP_IO *io, GP_ProgressCallback *callback)
{
	GP_Context *res = NULL;
	GP_PixelType pixel_type;
	struct pcx_header header;
	unsigned int w, h;
	int err = 0;

	uint16_t pcx_header[] = {
		0x0a,             /* creator ZSoft */
		GP_IO_BYTE,       /* version */
		0x01,             /* compression 1 == RLE */
		GP_IO_BYTE,       /* bpp */
		GP_IO_L2,         /* xs */
		GP_IO_L2,         /* ys */
		GP_IO_L2,         /* xe */
		GP_IO_L2,         /* ye */
		GP_IO_L2,         /* hres */
		GP_IO_L2,         /* vres */
		GP_IO_ARRAY | 48, /* 16 bit RGB palette */
		GP_IO_I1,         /* reserved */
		GP_IO_BYTE,       /* number of planes */
		GP_IO_L2,         /* bytes per line */
		GP_IO_L2,         /* palette info */
		GP_IO_IGN | 58,   /* filler to 128 bytes */
		GP_IO_END,
	};

	if (GP_IOReadF(io, pcx_header, &header.ver, &header.bpp,
	               &header.xs, &header.ys, &header.xe, &header.ye,
	               &header.hres, &header.vres,
	               header.palette, &header.nplanes,
	               &header.bytes_per_line, &header.pal_info) != 16) {
		GP_DEBUG(1, "Failed to read header: %s", strerror(errno));
		return NULL;
	}

	switch (header.ver) {
	case 0x00:
	case 0x02:
	case 0x03:
	case 0x04:
	case 0x05:
	break;
		GP_DEBUG(1, "Unknown version %x", header.ver);
		errno = EINVAL;
		return NULL;
	}

	GP_DEBUG(1, "Have PCX image ver=%x bpp=%"PRIu8" %"PRIu16"x%"PRIu16
	         "-%"PRIu16"x%"PRIu16" bytes_per_line=%"PRIu16
	         " nplanes=%"PRIu16" hres=%"PRIu16" vres=%"PRIu16,
	         header.ver, header.bpp, header.xs,
	         header.ys, header.xe, header.ye,
	         header.bytes_per_line, header.nplanes,
	         header.hres, header.vres);

	pixel_type = match_pixel_type(&header);

	if (pixel_type == GP_PIXEL_UNKNOWN) {
		GP_DEBUG(1, "Failed to match pixel type");
		err = ENOSYS;
		goto err0;
	}

	if (header.xs > header.xe || header.ys > header.ye) {
		GP_WARN("Invalid size %"PRIu16"-%"PRIu16"x%"PRIu16"-%"PRIu16,
		        header.xe, header.xs, header.ye, header.xs);
		err = EINVAL;
		goto err0;
	}

	w = header.xe - header.xs + 1;
	h = header.ye - header.ys + 1;

	res = GP_ContextAlloc(w, h, pixel_type);

	if (!res) {
		GP_DEBUG(1, "Malloc failed :(");
		err = ENOMEM;
		goto err0;
	}

	if ((err = read_image(io, &header, res, callback)))
		goto err1;

	GP_ProgressCallbackDone(callback);
	return res;
err1:
	GP_ContextFree(res);
err0:
	errno = err;
	return NULL;
}

GP_Context *GP_LoadPCX(const char *src_path, GP_ProgressCallback *callback)
{
	GP_IO *io;
	GP_Context *res;
	int err;

	io = GP_IOFile(src_path, GP_IO_RDONLY);
	if (!io)
		return NULL;

	res = GP_ReadPCX(io, callback);

	err = errno;
	GP_IOClose(io);
	errno = err;

	return res;
}
