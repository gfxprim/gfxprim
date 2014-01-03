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
#include <GP.h>

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
	GP_Backend *b;
	GP_Context *img;
	GP_IO *io;

	io = GP_IOMem(pgm, sizeof(pgm), NULL);

	if (!io) {
		fprintf(stderr, "Failed to initialize IO\n");
		return 1;
	}

	img = GP_ReadPGM(io, NULL);
	GP_IOClose(io);

	if (!img) {
		fprintf(stderr, "Failed to load image\n");
		return 1;
	}

	b = GP_BackendX11Init(NULL, 0, 0, WIN_W, WIN_H, "IO Example", 0);

	if (!b) {
		fprintf(stderr, "Failed to initialize backend\n");
		return 1;
	}

	GP_Fill(b->context, 0);
	GP_Blit_Clipped(img, 0, 0, img->w, img->h, b->context,
	                (WIN_W - img->w)/2, (WIN_H - img->h)/2);
	GP_BackendFlip(b);

	for (;;) {
		GP_Event ev;

		GP_BackendWaitEvent(b, &ev);

		switch (ev.type) {
		case GP_EV_KEY:
			switch (ev.val.val) {
			case GP_KEY_ESC:
			case GP_KEY_Q:
				GP_BackendExit(b);
				return 0;
			break;
			}
		break;
		case GP_EV_SYS:
			switch (ev.code) {
			case GP_EV_SYS_RESIZE:
			case GP_EV_SYS_QUIT:
				GP_BackendExit(b);
				return 0;
			break;
			}
		break;
		}
	}

	GP_BackendExit(b);
	return 0;
}
