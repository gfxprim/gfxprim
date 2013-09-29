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

  BMP RLE decoder.

 */

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
};

#define DECLARE_RLE(name, iw, ih) struct RLE name = { \
	.state = RLE_START, \
	.w = iw, .h = ih, \
	.x = 0, .y = 0, \
	.rep = 0\
}

#define GETC(f, rle) do {   \
	rle->c = fgetc(f);  \
                            \
	if (rle->c == EOF)  \
		return EIO; \
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

static int RLE8_repeat(FILE *f, uint8_t rep, struct RLE *rle)
{
	GETC(f, rle);

	GP_DEBUG(4, "RLE Repeat %i x 0x%02x", rep, rle->c);

	rle->rep = rep;
	rle->state = RLE_REPEAT;

	return 0;
}

static int RLE8_offset(FILE *f, struct RLE *rle)
{
	int x, y;

	x = fgetc(f);
	y = fgetc(f);

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

static int RLE8_next_undecoded(FILE *f, struct RLE *rle)
{
	GETC(f, rle);

	RLE8_move(rle);

	//GP_DEBUG(4, "RLE unencoded %u %u -> %02x", rle->x, rle->y, rle->c);

	if (--rle->rep == 0) {
		rle->state = RLE_START;
		/* must be padded to odd number of bytes */
		if (rle->flag)
			fgetc(f);
	}

	rle->move = 1;

	return 0;
}

static int RLE8_next_repeat(struct RLE *rle)
{
	RLE8_move(rle);

	//GP_DEBUG(4, "RLE repeat %u %u -> %02x", rle->x, rle->y, rle->c);

	if (--rle->rep == 0)
		rle->state = RLE_START;

	rle->move = 1;

	return 0;
}

static int RLE8_esc(FILE *f, struct RLE *rle)
{
	GETC(f, rle);

	GP_DEBUG(4, "RLE ESC %02x", rle->c);

	switch (rle->c) {
	case 0x00:
		return RLE8_end_of_scanline(rle);
	case 0x01:
		return RLE8_end_of_bitmap(rle);
	case 0x02:
		return RLE8_offset(f, rle);
	/* Undecoded sequence */
	default:
		GP_DEBUG(4, "RLE Undecoded x %i", rle->c);
		rle->state = RLE_UNDECODED;
		rle->rep = rle->c; 
		rle->flag = rle->c % 2;
		return 0;
	}
}

static int RLE8_start(FILE *f, struct RLE *rle)
{
	GETC(f, rle);

	switch (rle->c) {
	case 0x00:
		return RLE8_esc(f, rle);
	default:
		return RLE8_repeat(f, rle->c, rle);
	}
}

static int RLE8_next(FILE *f, struct RLE *rle)
{
	int err;

	for (;;) {
		switch (rle->state) {
		case RLE_START:
			if ((err = RLE8_start(f, rle)))
				return err;
		break;
		case RLE_REPEAT:
			return RLE8_next_repeat(rle);
		case RLE_UNDECODED:
			return RLE8_next_undecoded(f, rle);
		case RLE_STOP:
			return 0;
		default:
			/* Shouldn't be reached */
			GP_BUG("Invalid RLE state %u", rle->state);
			return EINVAL;
		}
	}
}

static int read_RLE8(FILE *f, struct bitmap_info_header *header,
                     GP_Context *context, GP_ProgressCallback *callback)
{
	uint32_t palette_size = get_palette_size(header);
	GP_Pixel palette[get_palette_size(header)];
	DECLARE_RLE(rle, header->w, GP_ABS(header->h));
	int err;

	if ((err = read_bitmap_palette(f, header, palette)))
		return err;

	if ((err = seek_pixels_offset(header, f)))
		return err;

	int cnt = 0;

	uint32_t x, y;

	/*
	 * Fill the image with first palette color.
	 *
	 * TODO: Untouched pixels should be treated as
	 *       1 bit transpanrency (in header3+)
	 */
	for (y = 0; y < context->h; y++) {
		for (x = 0; x < context->w; x++)
			GP_PutPixel_Raw_24BPP(context, x, y, palette[0]);
	}

	for (;;) {
		if ((err = RLE8_next(f, &rle)))
			return err;

		if (rle.state == RLE_STOP)
			break;

		GP_Pixel p;

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

		GP_PutPixel_Raw_24BPP(context, rle.x, ry, p);

		if (cnt++ > header->w) {
			cnt = 0;
			if (GP_ProgressCallbackReport(callback, rle.y,
			                              context->h, context->w)) {
				GP_DEBUG(1, "Operation aborted");
				return ECANCELED;
			}
		}
	}

	GP_ProgressCallbackDone(callback);
	return 0;
}
