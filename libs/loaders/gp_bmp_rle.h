// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2009-2014 Cyril Hrubis <metan@ucw.cz>
 */

/*

  BMP RLE decoder.

 */

#include "core/gp_fill.h"

enum RLE_state {
	RLE_START,
	/* end of RLE or bitmap was reached */
	RLE_STOP,
	/* in the middle of repeat sequence */
	RLE_REPEAT,
	/* in the middle of undecoded run */
	RLE_UNDECODED,
};

struct RLE {
	/* current state */
	uint8_t state;

	/* image size */
	uint32_t w;
	uint32_t h;

	/* current position */
	uint32_t x;
	uint32_t y;

	uint8_t rep;
	uint8_t flag:1;
	/* set to 1 if next pixel is in c */
	uint8_t move:2;

	/* current value */
	int c;

	gp_io *io;

	int buf_pos;
	int buf_end;
	uint8_t buf[512];
};

#define DECLARE_RLE(name, iw, ih, iio) struct RLE name = { \
	.state = RLE_START, \
	.w = iw, .h = ih,   \
	.x = 0, .y = 0,     \
	.rep = 0,           \
	.io = io,           \
	.buf_pos = 0,       \
	.buf_end = 0,       \
}

#define GETC(rle) do {   \
	if (rle->buf_pos < rle->buf_end) { \
		rle->c = rle->buf[rle->buf_pos++]; \
	} else { \
		rle->buf_end = gp_io_read(rle->io, rle->buf, sizeof(rle->buf));\
		if (rle->buf_end <= 0) \
			return EIO; \
		rle->c = rle->buf[0]; \
		rle->buf_pos = 1; \
	} \
} while (0)

static void RLE8_move(struct RLE *rle)
{
	if (!rle->move)
		return;

	if (++rle->x >= rle->w) {
		/* wrap around the end of line */
		rle->x = 0;

		/* all pixels filled */
		if (++rle->y >= rle->h) {
			GP_DEBUG(4, "y >= h, stop");
			rle->state = RLE_STOP;
		}
	}

	//GP_DEBUG(4, "RLE Move to %u %u", rle->x, rle->y);

	rle->move = 0;
}

static int RLE8_end_of_scanline(struct RLE *rle)
{
	GP_DEBUG(4, "End of scanline at %u %u", rle->x, rle->y);

	rle->x = 0;
	rle->y++;
	rle->move = 0;

	if (rle->y >= rle->h) {
		GP_DEBUG(4, "y >= h, stop");
		rle->state = RLE_STOP;
	}

	return 0;
}

static int RLE8_end_of_bitmap(struct RLE *rle)
{
	GP_DEBUG(4, "End of bitmap data");

	rle->state = RLE_STOP;

	return 0;
}

static int RLE8_repeat(uint8_t rep, struct RLE *rle)
{
	GETC(rle);

	GP_DEBUG(4, "RLE Repeat %i x 0x%02x", rep, rle->c);

	rle->rep = rep;
	rle->state = RLE_REPEAT;

	return 0;
}

static int RLE8_offset(struct RLE *rle)
{
	int x, y;

	GETC(rle);
	x = rle->c;
	GETC(rle);
	y = rle->c;

	if (x == EOF || y == EOF)
		return EIO;

	GP_DEBUG(1, "RLE offset %i %i", x, y);

	if (rle->x + (uint32_t)x >= rle->w || rle->y + (uint32_t)y >= rle->h) {
		GP_DEBUG(1, "RLE offset out of image, stop");
		rle->state = RLE_STOP;
	}

	rle->x += x;
	rle->y += y;

	return 0;
}

static int RLE8_next_undecoded(struct RLE *rle)
{
	GETC(rle);

	//GP_DEBUG(4, "RLE unencoded %u %u -> %02x", rle->x, rle->y, rle->c);

	if (--rle->rep == 0) {
		rle->state = RLE_START;
		/* must be padded to odd number of bytes */
		if (rle->flag)
			GETC(rle);
	}

	RLE8_move(rle);

	rle->move = 1;

	return 0;
}

static int RLE8_next_repeat(struct RLE *rle)
{
	//GP_DEBUG(4, "RLE repeat %u %u -> %02x", rle->x, rle->y, rle->c);

	if (--rle->rep == 0)
		rle->state = RLE_START;

	RLE8_move(rle);

	rle->move = 1;

	return 0;
}

static int RLE8_esc(struct RLE *rle)
{
	GETC(rle);

	GP_DEBUG(4, "RLE ESC %02x", rle->c);

	switch (rle->c) {
	case 0x00:
		return RLE8_end_of_scanline(rle);
	case 0x01:
		return RLE8_end_of_bitmap(rle);
	case 0x02:
		return RLE8_offset(rle);
	/* Undecoded sequence */
	default:
		GP_DEBUG(4, "RLE Undecoded x %i", rle->c);
		rle->state = RLE_UNDECODED;
		rle->rep = rle->c;
		rle->flag = rle->c % 2;
		return 0;
	}
}

static int RLE8_start(struct RLE *rle)
{
	GETC(rle);

	switch (rle->c) {
	case 0x00:
		return RLE8_esc(rle);
	default:
		return RLE8_repeat(rle->c, rle);
	}
}

static int RLE8_next(struct RLE *rle)
{
	int err;

	for (;;) {
		switch (rle->state) {
		case RLE_START:
			if ((err = RLE8_start(rle)))
				return err;
		break;
		case RLE_REPEAT:
			return RLE8_next_repeat(rle);
		case RLE_UNDECODED:
			return RLE8_next_undecoded(rle);
		case RLE_STOP:
			return 0;
		default:
			/* Shouldn't be reached */
			GP_BUG("Invalid RLE state %u", rle->state);
			return EINVAL;
		}
	}
}

static int read_RLE8(gp_io *io, struct gp_bmp_info_header *header,
                     gp_pixmap *pixmap, gp_progress_cb *callback)
{
	uint32_t palette_size = get_palette_size(header);
	DECLARE_RLE(rle, header->w, GP_ABS(header->h), io);
	int err;

	if (pixmap->pixel_type != GP_PIXEL_RGB888) {
		GP_WARN("Corrupted BMP header! "
			"RLE8 is 24bit (RGB888) palette but header says %s",
		        gp_pixel_type_name(pixmap->pixel_type));
		return EINVAL;
	}

	gp_pixel *palette = gp_temp_alloc(palette_size * sizeof(gp_pixel));

	if ((err = read_bitmap_palette(io, header, palette, palette_size)))
		goto err;

	if ((err = seek_pixels_offset(io, header)))
		goto err;

	int cnt = 0;

	/*
	 * Fill the image with first palette color.
	 *
	 * TODO: Untouched pixels should be treated as
	 *       1 bit transpanrency (in header3+)
	 */
	gp_fill(pixmap, palette[0]);

	for (;;) {
		if ((err = RLE8_next(&rle)))
			goto err;

		if (rle.state == RLE_STOP)
			break;

		gp_pixel p;

		uint8_t idx = rle.c;

		if (idx >= palette_size) {
			GP_DEBUG(1, "Index out of palette, ignoring");
			p = 0;
		} else {
			p = palette[idx];
		}

		int32_t ry;

		if (header->h < 0)
			ry = rle.y;
		else
			ry = GP_ABS(header->h) - 1 - rle.y;

		gp_putpixel_raw_24BPP(pixmap, rle.x, ry, p);

		if (cnt++ > header->w) {
			cnt = 0;
			if (gp_progress_cb_report(callback, rle.y,
			                              pixmap->h, pixmap->w)) {
				GP_DEBUG(1, "Operation aborted");
				err = ECANCELED;
				goto err;
			}
		}
	}

	gp_progress_cb_done(callback);
err:
	gp_temp_free(palette_size * sizeof(gp_pixel), palette);
	return err;
}
