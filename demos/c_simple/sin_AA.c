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

  Simple example that shows HLineAA() usage.

 */

#include <stdio.h>
#include <errno.h>
#include <string.h>

#include <GP.h>

static void redraw(GP_Pixmap *pixmap)
{
	static float param = 1;
	static float param2 = 0.01;
	static int flag = 1;
	GP_Pixel b = GP_RGBToPixmapPixel(0xbe, 0xbe, 0x9e, pixmap);
	unsigned int y;

	GP_Fill(pixmap, b);

	for (y = 0; y < pixmap->w; y++) {
		GP_Coord x0, x1, l1, l2;

		x0 = (pixmap->w)<<7;
		x1 = (pixmap->w)<<7;

		l1 = (pixmap->w)<<5;
		l2 = (pixmap->w)<<3;

		GP_Pixel p = GP_RGBToPixmapPixel(120 - 3 * param, abs(40 * param), 0, pixmap);

		l2 *= 4.00 * y / pixmap->h;

		l1 *= param;

		x0 += l1 * sin(param2 * y) + l2;
		x1 -= l1 * cos(param2 * y) + l2;

		GP_HLineAA(pixmap, x0, x1, y<<8, p);
	}

	if (flag) {
		param -= 0.02;

		if (param <= -2.40) {
			flag = 0;
			param2 += 0.01;

			if (param2 > 0.02)
				param2 = 0.01;
		}
	} else {
		param += 0.02;

		if (param >= 2.40)
			flag = 1;
	}
}

int main(void)
{
	GP_Backend *backend;
	static int pause_flag = 0;

	/* Initalize backend */
	backend = GP_BackendX11Init(NULL, 0, 0, 800, 600, "sin AA", 0);

	if (backend == NULL) {
		fprintf(stderr, "Failed to initalize backend\n");
		return 1;
	}

	/* Wait for events */
	for (;;) {
		if (!pause_flag) {
			redraw(backend->pixmap);
			GP_BackendFlip(backend);
		}

		GP_BackendPoll(backend);

		GP_Event ev;

		while (GP_BackendGetEvent(backend, &ev)) {
			if (ev.type == GP_EV_KEY && ev.code == GP_EV_KEY_DOWN) {
				switch (ev.val.val) {
				case GP_KEY_ESC:
				case GP_KEY_Q:
					GP_BackendExit(backend);
					return 0;
				case GP_KEY_P:
					pause_flag = !pause_flag;
				break;
				}
			}
		}

		usleep(10000);
	}

	return 0;
}
