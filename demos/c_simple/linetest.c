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
 * Copyright (C) 2009-2010 Jiri "BlueBear" Dluhos                            *
 *                         <jiri.bluebear.dluhos@gmail.com>                  *
 *                                                                           *
 * Copyright (C) 2009-2013 Cyril Hrubis <metan@ucw.cz>                       *
 *                                                                           *
 *****************************************************************************/

#include <math.h>

#include <GP.h>

/* Values for color pixels in display format. */
static GP_Pixel black, white;

static double start_angle = 0.0;

static int aa_flag = 0;
static int pause_flag = 0;

static GP_Backend *win;

void redraw_screen(void)
{
	double angle;
	int x, y;
	int w = win->context->w;
	int h = win->context->h;
	int xcenter = w/2;
	int ycenter = h/2;

	GP_Fill(win->context, black);

	for (angle = 0.0; angle < 2*M_PI; angle += 0.1) {
		x = (int) (w/2 * cos(start_angle + angle));
		y = (int) (h/2 * sin(start_angle + angle));

		int r = 127.0 + 127.0 * cos(start_angle + angle);
		int b = 127.0 + 127.0 * sin(start_angle + angle);

		GP_Pixel pixel;
		pixel = GP_RGBToPixel(r, 0, b, win->context->pixel_type);

		if (aa_flag) {
			GP_LineAA_Raw(win->context, GP_FP_FROM_INT(xcenter), GP_FP_FROM_INT(ycenter),
				GP_FP_FROM_INT(xcenter + x), GP_FP_FROM_INT(ycenter + y), pixel);
		} else {
			GP_Line(win->context, xcenter + x, ycenter + y, xcenter, ycenter, pixel);
			GP_Line(win->context, xcenter, ycenter, xcenter + x, ycenter + y, pixel);
		}
	}

	GP_BackendFlip(win);

	/* axes */
//	GP_HLineXYW(&context, 0, ycenter, display->w, white);
//	GP_VLineXYH(&context, xcenter, 0, display->h, white);
}

void event_loop(void)
{
	GP_Event ev;

	while (GP_BackendGetEvent(win, &ev)) {
		switch (ev.type) {
		case GP_EV_KEY:
			if (ev.code != GP_EV_KEY_DOWN)
				continue;

			switch (ev.val.key.key) {
			case GP_KEY_A:
				aa_flag = !aa_flag;
			break;
			case GP_KEY_ESC:
				GP_BackendExit(win);
				exit(0);
			break;
			case GP_KEY_P:
				pause_flag = !pause_flag;
			break;
			}
		break;
		case GP_EV_SYS:
			switch(ev.code) {
			case GP_EV_SYS_QUIT:
				GP_BackendExit(win);
				exit(0);
			break;
			case GP_EV_SYS_RESIZE:
				GP_BackendResizeAck(win);
			break;
			}
		break;
		}
	}
}

int main(int argc, char *argv[])
{
	const char *backend_opts = "X11";
	int opt;

	while ((opt = getopt(argc, argv, "b:")) != -1) {
		switch (opt) {
		case 'b':
			backend_opts = optarg;
		break;
		default:
			fprintf(stderr, "Invalid paramter '%c'\n", opt);
		}
	}

	win = GP_BackendInit(backend_opts, "Line Test");

	if (win == NULL) {
		fprintf(stderr, "Failed to initalize backend '%s'\n",
		        backend_opts);
		return 1;
	}

	white = GP_RGBToContextPixel(0xff, 0xff, 0xff, win->context);
	black = GP_RGBToContextPixel(0x00, 0x00, 0x00, win->context);

	redraw_screen();

	for (;;) {
		GP_BackendPoll(win);
		event_loop();

		usleep(20000);

		if (pause_flag)
			continue;

		redraw_screen();
		GP_BackendFlip(win);

		start_angle += 0.01;
		if (start_angle > 2*M_PI) {
			start_angle = 0.0;
		}
	}
}
