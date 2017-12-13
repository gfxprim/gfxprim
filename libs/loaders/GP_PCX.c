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

#include <loaders/GP_Loaders.gen.h>

/*
 * PCX RLE I/O Stream
 */
struct rle {
	/* RLE internal state */
	int cnt;
	int val;

	/* Read counter */
	off_t pos;

	/* Source I/O stream */
	gp_io *io;

	/* Read buffer */
	ssize_t buf_fill;
	ssize_t buf_pos;
	uint8_t buf[128];
};

static int rle_getc(struct rle *rle)
{
	if (rle->buf_pos < rle->buf_fill)
		return rle->buf[rle->buf_pos++];

	rle->buf_fill = gp_io_read(rle->io, rle->buf, sizeof(rle->buf));

	if (rle->buf_fill <= 0)
		return -1;

	rle->buf_pos = 1;
	return rle->buf[0];
}

/*
 * RLE I/O stream
 *
 * - reads exactly size bytes unless read from underlying I/O has failed
 *
 * - the size is expected to be multiple of bytes_per_line, the specification
 *   forbids for RLE to span across pixel lines, but there are images that does
 *   so. This code only prints a warning in this case.
 */
static ssize_t rle_read(gp_io *self, void *buf, size_t size)
{
	struct rle *priv = GP_IO_PRIV(self);
	unsigned int read = 0;
	uint8_t *bbuf = buf;
	int b;

	for (;;) {
		while (priv->cnt > 0 && read < size) {
			bbuf[read++] = priv->val;
			priv->cnt--;
		}

		if (read >= size) {
			priv->pos += read;
			if (priv->cnt)
				GP_WARN("Nonzero repeat count (%u) %02x at %zi",
				        priv->cnt, priv->val, priv->pos);
			//priv->cnt = 0;
			return read;
		}

		b = rle_getc(priv);

		if (b < 0)
			return priv->buf_fill;

		if ((b & 0xc0) == 0xc0) {
			priv->cnt = b & 0x3f;
			priv->val = rle_getc(priv);
			if (priv->val < 0)
				return priv->buf_fill;
		} else {
			priv->cnt = 1;
			priv->val = b;
		}
	}
}

/*
 * Only seeks forward to skip padding also works for gp_io_tell().
 */
static off_t rle_seek(gp_io *self, off_t off, enum gp_io_whence whence)
{
	uint8_t b;
	struct rle *priv = GP_IO_PRIV(self);

	if (whence != GP_IO_SEEK_CUR || off < 0)
		return EINVAL;

	while (off--)
		rle_read(self, &b, 1);

	return priv->pos;
}

static int rle_close(gp_io *self)
{
	free(self);
	return 0;
}

static gp_io *rle(gp_io *io)
{
	gp_io *rle = malloc(sizeof(gp_io) + sizeof(struct rle));

	if (!rle)
		return NULL;

	struct rle *priv = GP_IO_PRIV(rle);

	priv->cnt = 0;
	priv->buf_fill = 0;
	priv->buf_pos = 0;
	priv->io = io;

	rle->read = rle_read;
	rle->write = NULL;
	rle->seek = rle_seek;
	rle->close = rle_close;

	return rle;
}

/* Creator ZSoft: 0x0a
 * Version:       0x00, 0x02, 0x03, 0x04, 0x05
 * RLE:           0x01
 * BPP:           0x01, 0x02, 0x04, 0x08
 */
int gp_match_pcx(const void *buf)
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

#include "core/GP_BitSwap.h"

static int read_g1(gp_io *io, struct pcx_header *header,
                   gp_pixmap *res, gp_progress_cb *callback)
{
	uint32_t y;
	int padd = (int)header->bytes_per_line - (int)res->bytes_per_row;

	if (padd < 0) {
		GP_WARN("Invalid number of bytes per line");
		return EINVAL;
	}

	gp_io *rle_io = rle(io);
	if (!rle_io)
		return errno;

	for (y = 0; y < res->h; y++) {
		uint8_t *addr = GP_PIXEL_ADDR(res, 0, y);
		gp_io_read(rle_io, addr, res->bytes_per_row);
		gp_io_seek(rle_io, GP_IO_SEEK_CUR, padd);

		//TODO: FIX Endians
		gp_bit_swap_row_b1(addr, res->bytes_per_row);

		if (gp_progress_cb_report(callback, y, res->h, res->w)) {
			GP_DEBUG(1, "Operation aborted");
			gp_io_close(rle_io);
			return ECANCELED;
		}
	}

	gp_io_close(rle_io);
	return 0;
}

static int read_rgb888(gp_io *io, struct pcx_header *header,
                       gp_pixmap *res, gp_progress_cb *callback)
{
	uint32_t x, y;
	unsigned int bpr = header->bytes_per_line;
	uint8_t b[3 * bpr];

	gp_io *rle_io = rle(io);
	if (!rle_io)
		return errno;

	for (y = 0; y < res->h; y++) {
		//readline(io, b, sizeof(b), 0);

		gp_io_read(rle_io, b, sizeof(b));

		for (x = 0; x < res->w; x++) {
			gp_pixel pix = GP_PIXEL_CREATE_RGB888(b[x],
			                                      b[x+bpr],
			                                      b[x+2*bpr]);
			gp_putpixel_raw_24BPP(res, x, y, pix);
		}

		if (gp_progress_cb_report(callback, y, res->h, res->w)) {
			GP_DEBUG(1, "Operation aborted");
			gp_io_close(rle_io);
			return ECANCELED;
		}
	}

	gp_io_close(rle_io);
	return 0;
}

static int read_16_palette(gp_io *io, struct pcx_header *header,
                           gp_pixmap *res, gp_progress_cb *callback)
{
	uint32_t x, y;
	unsigned int i;
	uint8_t b[header->bytes_per_line];
	gp_pixel palette[16];
	uint8_t idx = 0, mask, mod;

	for (i = 0; i < 16; i++) {
		palette[i] = (gp_pixel)header->palette[3*i] << 16;
		palette[i] |= (gp_pixel)header->palette[3*i+1] << 8;
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

	gp_io *rle_io = rle(io);
	if (!rle_io)
		return errno;

	for (y = 0; y < res->h; y++) {
		gp_io_read(rle_io, b, sizeof(b));

		i = 0;

		for (x = 0; x < res->w; x++) {

			if (!(x % mod))
				idx = b[i++];

			gp_putpixel_raw_24BPP(res, x, y, palette[(idx & mask) >> (8 - header->bpp)]);
			idx <<= header->bpp;
		}

		if (gp_progress_cb_report(callback, y, res->h, res->w)) {
			GP_DEBUG(1, "Operation aborted");
			gp_io_close(rle_io);
			return ECANCELED;
		}
	}

	gp_io_close(rle_io);
	return 0;
}

#define PALETTE_SIZE (3 * 256 + 1)

static int read_256_palette(gp_io *io, struct pcx_header *header,
                            gp_pixmap *res, gp_progress_cb *callback)
{
	uint32_t x, y;
	unsigned int i;
	uint8_t buf[GP_MAX(PALETTE_SIZE, header->bytes_per_line)];
	gp_pixel palette[256];

	if (gp_io_seek(io, -769, GP_IO_SEEK_END) == (off_t)-1) {
		GP_DEBUG(1, "Failed to seek to palette: %s", strerror(errno));
		return EIO;
	}

	if (gp_io_fill(io, buf, PALETTE_SIZE)) {
		GP_DEBUG(1, "Failed to read palette: %s", strerror(errno));
		return EIO;
	}

	if (buf[0] != 0x0c) {
		GP_DEBUG(1, "Wrong palette marker");
		return EIO;
	}

	for (i = 0; i < 256; i++)
		palette[i] = (buf[3*i+1]<<16) | (buf[3*i+2])<<8 | buf[3*i+3];

	if (gp_io_seek(io, 128, GP_IO_SEEK_SET) == (off_t)-1) {
		GP_DEBUG(1, "Failed to seek to image data: %s",
		         strerror(errno));
		return EIO;
	}

	gp_io *rle_io = rle(io);
	if (!rle_io)
		return errno;

	for (y = 0; y < res->h; y++) {
		gp_io_read(rle_io, buf, header->bytes_per_line);

		for (x = 0; x < res->w; x++)
			gp_putpixel_raw_24BPP(res, x, y, palette[buf[x]]);

		if (gp_progress_cb_report(callback, y, res->h, res->w)) {
			GP_DEBUG(1, "Operation aborted");
			gp_io_close(rle_io);
			return ECANCELED;
		}
	}

	gp_io_close(rle_io);
	return 0;
}

static gp_pixel_type match_pixel_type(struct pcx_header *header)
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

static int read_image(gp_io *io, struct pcx_header *header,
                      gp_pixmap *res, gp_progress_cb *callback)
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
	       gp_pixel_type_name(res->pixel_type));
	return ENOSYS;
}

static void fill_metadata(struct pcx_header *header, gp_storage *storage)
{
	gp_storage_add_int(storage, NULL, "Width", header->xe - header->xs + 1);
	gp_storage_add_int(storage, NULL, "Height", header->ye - header->ys + 1);
	gp_storage_add_int(storage, NULL, "Version", header->ver);
	gp_storage_add_int(storage, NULL, "Bits per Sample", header->bpp);
	gp_storage_add_int(storage, NULL, "Samples per Pixel", header->nplanes);
}

int gp_read_pcx_ex(gp_io *io, gp_pixmap **img, gp_storage *storage,
                 gp_progress_cb *callback)
{
	gp_pixmap *res = NULL;
	gp_pixel_type pixel_type;
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

	if (gp_io_readf(io, pcx_header, &header.ver, &header.bpp,
	                &header.xs, &header.ys, &header.xe, &header.ye,
	                &header.hres, &header.vres,
	                header.palette, &header.nplanes,
	                &header.bytes_per_line, &header.pal_info) != 16) {
		GP_DEBUG(1, "Failed to read header: %s", strerror(errno));
		return 1;
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
		return 1;
	}

	GP_DEBUG(1, "Have PCX image ver=%x bpp=%"PRIu8" %"PRIu16"x%"PRIu16
	         "-%"PRIu16"x%"PRIu16" bytes_per_line=%"PRIu16
	         " nplanes=%"PRIu16" hres=%"PRIu16" vres=%"PRIu16,
	         header.ver, header.bpp, header.xs,
	         header.ys, header.xe, header.ye,
	         header.bytes_per_line, header.nplanes,
	         header.hres, header.vres);

	if (storage)
		fill_metadata(&header, storage);

	if (!img)
		return 0;

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

	uint32_t max_w = ((uint32_t)header.bytes_per_line * 8) / header.bpp;

	if (w > max_w) {
		GP_WARN("Truncating image width (%u) to "
		        "bytes_per_line * 8 / bpp (%"PRIu32")", w, max_w);
		w = max_w;
	}

	res = gp_pixmap_alloc(w, h, pixel_type);

	if (!res) {
		GP_DEBUG(1, "Malloc failed :(");
		err = ENOMEM;
		goto err0;
	}

	if ((err = read_image(io, &header, res, callback)))
		goto err1;

	gp_progress_cb_done(callback);

	*img = res;
	return 0;
err1:
	gp_pixmap_free(res);
err0:
	errno = err;
	return 1;
}

const gp_loader gp_pcx = {
	.Read = gp_read_pcx_ex,
	.Match = gp_match_pcx,

	.fmt_name = "ZSoft PCX",
	.extensions = {"pcx", NULL},
};
