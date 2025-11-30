// SPDX-License-Identifier: GPL-2.1-or-later
/*
 * Copyright (C) 2009-2010 Jiri "BlueBear" Dluhos
 *                         <jiri.bluebear.dluhos@gmail.com>
 *
 * Copyright (C) 2009-2025 Cyril Hrubis <metan@ucw.cz>
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <gfxprim.h>

static gp_backend *backend;

static gp_pixel red, green, white, black;

static void draw_event(gp_event *ev)
{
	gp_pixmap *win = backend->pixmap;
	static gp_size ksize = 0, bsize = 0;
	int align = GP_ALIGN_RIGHT|GP_VALIGN_BOTTOM;

	switch (ev->type) {
	case GP_EV_KEY:
		gp_text_clear(win, NULL, 20, 20, align, black, ksize);
		ksize = gp_print(win, NULL, 20, 20, align,
		                 white, black, "Key=%s %u UTF=0x%04x",
				 gp_ev_key_name(ev->key.key),
				 (unsigned int)ev->code,
				 ev->key.utf);
	break;
	case GP_EV_SYS:
		switch (ev->code) {
		case GP_EV_SYS_BACKLIGHT:
			gp_text_clear(win, NULL, 20, 60, align, black, bsize);
			bsize = gp_print(win, NULL, 20, 60, align, white, black,
			                 "Backlight brightness %i%%", ev->val);
		break;
		}
	break;
	default:
		return;
	}

	gp_backend_update(backend);
}

static void event_loop(void)
{
	static int size = 0;
	int align = GP_ALIGN_RIGHT|GP_VALIGN_BOTTOM;

	for (;;) {
		gp_backend_wait(backend);
		gp_pixmap *win = backend->pixmap;

		while (gp_backend_ev_queued(backend)) {
			gp_event *ev = gp_backend_ev_get(backend);

			gp_ev_dump(ev);

			switch (ev->type) {
			case GP_EV_KEY:
				draw_event(ev);

				switch (ev->key.key) {
				case GP_KEY_ESC:
					gp_backend_exit(backend);
					exit(0);
				break;
				case GP_BTN_LEFT:
					gp_hline_xxy(win, ev->st->cursor_x - 3,
					            ev->st->cursor_x + 3,
						    ev->st->cursor_y, red);
					gp_vline_xyy(win, ev->st->cursor_x,
					            ev->st->cursor_y - 3,
						    ev->st->cursor_y + 3, red);
					gp_backend_update(backend);
				break;
				default:
				break;
				}
			break;
			case GP_EV_REL:
				switch (ev->code) {
				case GP_EV_REL_POS:
					if (gp_ev_key_pressed(ev, GP_BTN_LEFT)) {
						gp_putpixel(win, ev->st->cursor_x, ev->st->cursor_y,
							    green);
					}

					gp_text_clear(win, NULL, 20, 60, align,
					              black, size);
					size = gp_print(win, NULL, 20, 60, align,
					                white, black, "X=%3u Y=%3u dX=%3i dY=%3i",
						        ev->st->cursor_x, ev->st->cursor_y,
							ev->rel.rx, ev->rel.ry);
					gp_backend_update(backend);
				break;
				case GP_EV_REL_WHEEL:
					gp_text_clear(win, NULL, 20, 60, align, black, size);
					size = gp_print(win, NULL, 20, 60, align, white, black,
							"wheel=%i", ev->val);
					gp_backend_update(backend);
				break;
				}
			break;
			case GP_EV_ABS:
				case GP_EV_ABS_POS:
					gp_putpixel(win, ev->st->cursor_x,
					            ev->st->cursor_y,
						    green);
					gp_backend_update(backend);
				break;
			break;
			case GP_EV_SYS:
				draw_event(ev);

				switch (ev->code) {
				case GP_EV_SYS_RESIZE:
					gp_backend_resize_ack(backend);
					gp_fill(backend->pixmap, black);
					gp_backend_update(backend);
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
	const char *backend_opts = NULL;
	int opt;

	while ((opt = getopt(argc, argv, "b:h")) != -1) {
		switch (opt) {
		case 'b':
			backend_opts = optarg;
		break;
		case 'h':
			printf("Options\n-------\n\n");
			printf("-b backend\n\n");
			printf("Backends\n--------\n\n");
			gp_backend_init_help();
			return 0;
		default:
			fprintf(stderr, "Invalid paramter '%c'\n", opt);
			return 1;
		}
	}

	backend = gp_backend_init(backend_opts, 0, 0, "Input Test");
	if (!backend) {
		fprintf(stderr, "Failed to initalize backend '%s'\n",
		        backend_opts);
		return 1;
	}

	gp_pixmap *win = backend->pixmap;

	red   = gp_rgb_to_pixmap_pixel(0xff, 0x00, 0x00, win);
	green = gp_rgb_to_pixmap_pixel(0x00, 0xff, 0x00, win);
	white = gp_rgb_to_pixmap_pixel(0xff, 0xff, 0xff, win);
	black = gp_rgb_to_pixmap_pixel(0x00, 0x00, 0x00, win);

	gp_fill(win, black);
	gp_backend_update(backend);

	event_loop();
	return 0;
}
