// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2024 Cyril Hrubis <metan@ucw.cz>
 */

#include <core/gp_pixmap.h>
#include <core/gp_get_put_pixel.h>
#include <backends/gp_cursor.h>

#define B(a, b, c, d) ((a) | ((b)<<2) | ((c)<<4) | (d<<6))

static uint8_t cursor_arrow_32[] = {
	B(1, 0, 0, 0), B(0, 0, 0, 0), B(0, 0, 0, 0),
	B(1, 1, 0, 0), B(0, 0, 0, 0), B(0, 0, 0, 0),
	B(1, 2, 1, 0), B(0, 0, 0, 0), B(0, 0, 0, 0),
	B(1, 2, 2, 1), B(0, 0, 0, 0), B(0, 0, 0, 0),
	B(1, 2, 2, 2), B(1, 0, 0, 0), B(0, 0, 0, 0),
	B(1, 2, 2, 2), B(2, 1, 0, 0), B(0, 0, 0, 0),
	B(1, 2, 2, 2), B(2, 2, 1, 0), B(0, 0, 0, 0),
	B(1, 2, 2, 2), B(2, 2, 2, 1), B(0, 0, 0, 0),
	B(1, 2, 2, 2), B(2, 2, 2, 2), B(1, 0, 0, 0),
	B(1, 2, 2, 2), B(2, 2, 2, 2), B(2, 1, 0, 0),
	B(1, 2, 2, 2), B(2, 2, 1, 1), B(1, 1, 0, 0),
	B(1, 2, 2, 1), B(2, 2, 1, 0), B(0, 0, 0, 0),
	B(1, 2, 1, 0), B(1, 2, 2, 1), B(0, 0, 0, 0),
	B(1, 1, 0, 0), B(1, 2, 2, 1), B(0, 0, 0, 0),
	B(0, 0, 0, 0), B(0, 1, 2, 2), B(1, 0, 0, 0),
	B(0, 0, 0, 0), B(0, 1, 2, 2), B(1, 0, 0, 0),
	B(0, 0, 0, 0), B(0, 0, 1, 1), B(0, 0, 0, 0),
};

static uint8_t cursor_i_beam_32[] = {
	B(2, 2, 2, 2), B(0, 2, 2, 2), B(2, 0, 0, 0),
	B(2, 1, 1, 1), B(2, 1, 1, 1), B(2, 0, 0, 0),
	B(2, 2, 2, 1), B(1, 1, 2, 2), B(2, 0, 0, 0),
	B(0, 0, 0, 2), B(1, 2, 0, 0), B(0, 0, 0, 0),
	B(0, 0, 0, 2), B(1, 2, 0, 0), B(0, 0, 0, 0),
	B(0, 0, 0, 2), B(1, 2, 0, 0), B(0, 0, 0, 0),
	B(0, 0, 0, 2), B(1, 2, 0, 0), B(0, 0, 0, 0),
	B(0, 0, 0, 2), B(1, 2, 0, 0), B(0, 0, 0, 0),
	B(0, 0, 0, 2), B(1, 2, 0, 0), B(0, 0, 0, 0),
	B(0, 0, 0, 2), B(1, 2, 0, 0), B(0, 0, 0, 0),
	B(0, 0, 0, 2), B(1, 2, 0, 0), B(0, 0, 0, 0),
	B(0, 0, 0, 2), B(1, 2, 0, 0), B(0, 0, 0, 0),
	B(0, 0, 0, 2), B(1, 2, 0, 0), B(0, 0, 0, 0),
	B(0, 0, 0, 2), B(1, 2, 0, 0), B(0, 0, 0, 0),
	B(2, 2, 2, 1), B(1, 1, 2, 2), B(2, 0, 0, 0),
	B(2, 1, 1, 1), B(2, 1, 1, 1), B(2, 0, 0, 0),
	B(2, 2, 2, 2), B(0, 2, 2, 2), B(2, 0, 0, 0),
};

static uint8_t cursor_crosshair_32[] = {
	B(0, 0, 0, 0), B(1, 1, 1, 0), B(0, 0, 0, 0),
	B(0, 0, 0, 0), B(1, 2, 1, 0), B(0, 0, 0, 0),
	B(0, 0, 0, 0), B(1, 2, 1, 0), B(0, 0, 0, 0),
	B(0, 0, 0, 0), B(1, 2, 1, 0), B(0, 0, 0, 0),
	B(1, 1, 1, 1), B(1, 2, 1, 1), B(1, 1, 1, 0),
	B(1, 2, 2, 2), B(2, 0, 2, 2), B(2, 2, 1, 0),
	B(1, 1, 1, 1), B(1, 2, 1, 1), B(1, 1, 1, 0),
	B(0, 0, 0, 0), B(1, 2, 1, 0), B(0, 0, 0, 0),
	B(0, 0, 0, 0), B(1, 2, 1, 0), B(0, 0, 0, 0),
	B(0, 0, 0, 0), B(1, 2, 1, 0), B(0, 0, 0, 0),
	B(0, 0, 0, 0), B(1, 1, 1, 0), B(0, 0, 0, 0),
};

static uint8_t cursor_hand_32[] = {
	B(0, 0, 0, 1), B(0, 0, 0, 0), B(0, 0, 0, 0),
	B(0, 0, 1, 2), B(1, 0, 0, 0), B(0, 0, 0, 0),
	B(0, 1, 2, 1), B(2, 1, 0, 0), B(0, 0, 0, 0),
	B(0, 1, 2, 1), B(2, 1, 1, 1), B(0, 0, 0, 0),
	B(0, 1, 2, 1), B(2, 2, 1, 2), B(1, 0, 0, 0),
	B(0, 1, 2, 1), B(2, 1, 2, 1), B(2, 1, 0, 0),
	B(0, 1, 2, 1), B(2, 1, 2, 1), B(2, 2, 1, 0),
	B(1, 2, 1, 1), B(2, 1, 2, 1), B(2, 1, 2, 1),
	B(1, 2, 1, 1), B(2, 1, 2, 1), B(2, 1, 2, 1),
	B(1, 2, 1, 1), B(1, 1, 1, 1), B(2, 1, 2, 1),
	B(1, 2, 1, 1), B(1, 1, 1, 1), B(1, 1, 2, 1),
	B(1, 2, 1, 1), B(1, 1, 1, 1), B(1, 1, 2, 1),
	B(1, 2, 1, 1), B(1, 1, 1, 1), B(1, 1, 2, 1),
	B(0, 1, 2, 1), B(1, 1, 1, 1), B(1, 2, 1, 0),
	B(0, 1, 2, 1), B(1, 1, 1, 1), B(1, 2, 1, 0),
	B(0, 0, 1, 2), B(2, 2, 2, 2), B(2, 1, 0, 0),
	B(0, 0, 1, 2), B(2, 2, 2, 2), B(2, 1, 0, 0),
	B(0, 0, 0, 1), B(1, 1, 1, 1), B(1, 0, 0, 0),
};

gp_cursor *gp_cursors_32[] = {
	/* Pointer arrow */
	&(gp_cursor){
		.x_off = 0,
		.y_off = 0,
		.w = 12,
		.h = 17,
		.pixmap = cursor_arrow_32,
	},
	/* I-beam text cursor */
	&(gp_cursor){
		.x_off = -5,
		.y_off = -8,
		.w = 12,
		.h = 18,
		.pixmap = cursor_i_beam_32,
	},
	/* Crosshair */
	&(gp_cursor){
		.x_off = -5,
		.y_off = -5,
		.w = 12,
		.h = 11,
		.pixmap = cursor_crosshair_32,
	},
	/* Hand */
	&(gp_cursor){
		.x_off = -3,
		.y_off = 0,
		.w = 12,
		.h = 18,
		.pixmap = cursor_hand_32,
	},
};

void gp_cursor_render(gp_cursor *cursor, gp_pixmap *dst,
                      gp_pixel fg, gp_pixel bg, uint32_t x_off, uint32_t y_off)
{
	unsigned int x, y;

	x_off += cursor->x_off;
	y_off += cursor->y_off;

	for (y = 0; y < cursor->h; y++) {
		for (x = 0; x < cursor->w/4; x++) {
			uint8_t byte = cursor->pixmap[x + y * cursor->w/4];

			switch (byte & 0x03) {
			case 0x01:
				gp_putpixel(dst, x_off + x*4, y_off+y, fg);
			break;
			case 0x02:
				gp_putpixel(dst, x_off + x*4, y_off+y, bg);
			break;
			}

			switch ((byte>>2) & 0x03) {
			case 0x01:
				gp_putpixel(dst, x_off + x*4+1, y_off+y, fg);
			break;
			case 0x02:
				gp_putpixel(dst, x_off + x*4+1, y_off+y, bg);
			break;
			}

			switch ((byte>>4) & 0x03) {
			case 0x01:
				gp_putpixel(dst, x_off + x*4+2, y_off+y, fg);
			break;
			case 0x02:
				gp_putpixel(dst, x_off + x*4+2, y_off+y, bg);
			break;
			}

			switch ((byte>>6) & 0x03) {
			case 0x01:
				gp_putpixel(dst, x_off + x*4+3, y_off+y, fg);
			break;
			case 0x02:
				gp_putpixel(dst, x_off + x*4+3, y_off+y, bg);
			break;
			}
		}
	}
}
