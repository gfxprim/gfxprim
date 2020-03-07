// SPDX-License-Identifier: GPL-2.1-or-later
/*
 * Copyright (C) 2009-2013 Cyril Hrubis <metan@ucw.cz>
 */

 /*

   Simple backend example.

  */

#include <stdio.h>
#include <gfxprim.h>

static void redraw(gp_backend *self)
{
	gp_pixmap *pixmap = self->pixmap;
	gp_pixel white_pixel, black_pixel;

	black_pixel = gp_rgb_to_pixmap_pixel(0x00, 0x00, 0x00, pixmap);
	white_pixel = gp_rgb_to_pixmap_pixel(0xff, 0xff, 0xff, pixmap);

	gp_fill(pixmap, black_pixel);
	gp_line(pixmap, 0, 0, pixmap->w - 1, pixmap->h - 1, white_pixel);
	gp_line(pixmap, 0, pixmap->h - 1, pixmap->w - 1, 0, white_pixel);

	/* Update the backend screen */
	gp_backend_flip(self);
}

int main(int argc, char *argv[])
{
	gp_backend *backend;
	const char *backend_opts = "X11:100x100";
	int opt;

	while ((opt = getopt(argc, argv, "b:h")) != -1) {
		switch (opt) {
		case 'b':
			backend_opts = optarg;
		break;
		case 'h':
			gp_backend_init(NULL, NULL);
			return 0;
		break;
		default:
			fprintf(stderr, "Invalid paramter '%c'\n", opt);
			return 1;
		}
	}

	backend = gp_backend_init(backend_opts, "Backend Example");

	if (backend == NULL) {
		fprintf(stderr, "Failed to initialize backend\n");
		return 1;
	}

	redraw(backend);

	/* Handle events */
	for (;;) {
		gp_event ev;

		gp_backend_wait_event(backend, &ev);

		gp_event_dump(&ev);

		switch (ev.type) {
		case GP_EV_KEY:
			switch (ev.val.val) {
			case GP_KEY_ESC:
			case GP_KEY_Q:
				gp_backend_exit(backend);
				return 0;
			break;
			}
		break;
		case GP_EV_SYS:
			switch (ev.code) {
			case GP_EV_SYS_RESIZE:
				gp_backend_resize_ack(backend);
				redraw(backend);
			break;
			case GP_EV_SYS_QUIT:
				gp_backend_exit(backend);
				return 0;
			break;
			}
		break;
		}
	}

	gp_backend_exit(backend);

	return 0;
}
