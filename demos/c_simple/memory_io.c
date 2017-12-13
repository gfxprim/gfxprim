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

   Simple memory IO example.

  */

#include <stdio.h>
#include <gfxprim.h>

/*
 * Binary PGM stored in an array
 */
static char pgm[] = {
	/* header */
	'P', '5', '\n',
	'1', '0', ' ', '1', '0', '\n',
	'2', '5', '5', '\n',
	/* data */
	0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00,
	0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00,
	0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0xff, 0xff,
	0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0xff, 0xff,
	0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00,
	0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00,
	0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0xff, 0xff,
	0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0xff, 0xff,
	0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00,
	0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00,
};

#define WIN_W 100
#define WIN_H 100

int main(void)
{
	gp_backend *b;
	gp_pixmap *img;
	gp_io *io;

	io = gp_io_mem(pgm, sizeof(pgm), NULL);

	if (!io) {
		fprintf(stderr, "Failed to initialize IO\n");
		return 1;
	}

	img = gp_read_pgm(io, NULL);
	gp_io_close(io);

	if (!img) {
		fprintf(stderr, "Failed to load image\n");
		return 1;
	}

	b = gp_x11_init(NULL, 0, 0, WIN_W, WIN_H, "IO Example", 0);

	if (!b) {
		fprintf(stderr, "Failed to initialize backend\n");
		return 1;
	}

	gp_fill(b->pixmap, 0);
	gp_blit_clipped(img, 0, 0, img->w, img->h, b->pixmap,
	                (WIN_W - img->w)/2, (WIN_H - img->h)/2);
	gp_backend_flip(b);

	for (;;) {
		gp_event ev;

		gp_backend_wait_event(b, &ev);

		switch (ev.type) {
		case GP_EV_KEY:
			switch (ev.val.val) {
			case GP_KEY_ESC:
			case GP_KEY_Q:
				gp_backend_exit(b);
				return 0;
			break;
			}
		break;
		case GP_EV_SYS:
			switch (ev.code) {
			case GP_EV_SYS_RESIZE:
			case GP_EV_SYS_QUIT:
				gp_backend_exit(b);
				return 0;
			break;
			}
		break;
		}
	}

	gp_backend_exit(b);
	return 0;
}
