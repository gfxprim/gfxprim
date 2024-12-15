// SPDX-License-Identifier: GPL-2.1-or-later
/*
 * Copyright (C) 2009-2014 Cyril Hrubis <metan@ucw.cz>
 */

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
		gp_event *ev = gp_backend_ev_wait(b);

		switch (ev->type) {
		case GP_EV_KEY:
			switch (ev->val) {
			case GP_KEY_ESC:
			case GP_KEY_Q:
				gp_backend_exit(b);
				return 0;
			break;
			}
		break;
		case GP_EV_SYS:
			switch (ev->code) {
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
