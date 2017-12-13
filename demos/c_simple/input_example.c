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

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "gfxprim.h"

static gp_pixmap *win;
static gp_backend *backend;

static gp_pixel red, green, white, black;

static void draw_event(gp_event *ev)
{
	static gp_size size = 0;

	if (ev->type != GP_EV_KEY)
		return;

	int align = GP_ALIGN_RIGHT|GP_VALIGN_BOTTOM;

	gp_text_clear(win, NULL, 20, 20, align, black, size);
	size = gp_print(win, NULL, 20, 20, align,
	                white, black, "Key=%s",
			gp_event_key_name(ev->val.key.key));

	gp_backend_flip(backend);
}

static void event_loop(void)
{
	for (;;) {
		gp_backend_wait(backend);

		while (gp_backend_events_queued(backend)) {
			gp_event ev;

			gp_backend_get_event(backend, &ev);
			gp_event_dump(&ev);

			switch (ev.type) {
			case GP_EV_KEY:
				draw_event(&ev);

				switch (ev.val.key.key) {
				case GP_KEY_ESC:
					gp_backend_exit(backend);
					exit(0);
				break;
				case GP_BTN_LEFT:
					gp_hline_xxy(win, ev.cursor_x - 3,
					            ev.cursor_x + 3,
						    ev.cursor_y, red);
					gp_vline_xyy(win, ev.cursor_x,
					            ev.cursor_y - 3,
						    ev.cursor_y + 3, red);
					gp_backend_flip(backend);
				break;
				default:
				break;
				}
			break;
			case GP_EV_REL:
				switch (ev.code) {
				static int size = 0;
				case GP_EV_REL_POS:
					if (gp_event_get_key(&ev, GP_BTN_LEFT)) {
						gp_putpixel(win, ev.cursor_x,
							    ev.cursor_y,
							    green);
					}
					int align = GP_ALIGN_RIGHT|GP_VALIGN_BOTTOM;

					gp_text_clear(win, NULL, 20, 40, align,
					              black, size);
					size = gp_print(win, NULL, 20, 40, align,
					                white, black, "X=%3u Y=%3u dX=%3i dY=%3i",
						        ev.cursor_x, ev.cursor_y,
							ev.val.rel.rx, ev.val.rel.ry);
					gp_backend_flip(backend);
				break;
				}
			break;
			case GP_EV_SYS:
				switch (ev.code) {
				case GP_EV_SYS_RESIZE:
					gp_backend_resize_ack(backend);
				break;
				case GP_EV_SYS_QUIT:
					gp_backend_exit(backend);
					exit(0);
				break;
				}
			break;
			}
		}
	}
}

int main(int argc, char *argv[])
{
	const char *backend_opts = "X11";
	int opt;

	while ((opt = getopt(argc, argv, "b:h")) != -1) {
		switch (opt) {
		case 'b':
			backend_opts = optarg;
		break;
		case 'h':
			gp_backend_init("help", NULL);
			return 0;
		break;
		default:
			fprintf(stderr, "Invalid paramter '%c'\n", opt);
			return 1;
		}
	}

	backend = gp_backend_init(backend_opts, "Input Test");

	if (backend == NULL) {
		fprintf(stderr, "Failed to initalize backend '%s'\n",
		        backend_opts);
		return 1;
	}

	win = backend->pixmap;

	red   = gp_rgb_to_pixmap_pixel(0xff, 0x00, 0x00, win);
	green = gp_rgb_to_pixmap_pixel(0x00, 0xff, 0x00, win);
	white = gp_rgb_to_pixmap_pixel(0xff, 0xff, 0xff, win);
	black = gp_rgb_to_pixmap_pixel(0x00, 0x00, 0x00, win);

	gp_fill(win, black);
	gp_backend_flip(backend);

	for (;;) {
		gp_backend_wait(backend);
		event_loop();
	}
}
