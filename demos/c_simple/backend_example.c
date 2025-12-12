// SPDX-License-Identifier: GPL-2.1-or-later
/*
 * Copyright (C) 2009-2026 Cyril Hrubis <metan@ucw.cz>
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

	if (!pixmap)
		return;

	black_pixel = gp_rgb_to_pixmap_pixel(0x00, 0x00, 0x00, pixmap);
	white_pixel = gp_rgb_to_pixmap_pixel(0xff, 0xff, 0xff, pixmap);

	gp_fill(pixmap, black_pixel);
	gp_line(pixmap, 0, 0, pixmap->w - 1, pixmap->h - 1, white_pixel);
	gp_line(pixmap, 0, pixmap->h - 1, pixmap->w - 1, 0, white_pixel);

	gp_backend_flip(self);
}

int main(int argc, char *argv[])
{
	gp_backend *backend;
	const char *backend_opts = NULL;
	int opt;

	while ((opt = getopt(argc, argv, "b:h")) != -1) {
		switch (opt) {
		case 'b':
			backend_opts = optarg;
		break;
		case 'h':
			gp_backend_init_help();
			return 0;
		break;
		default:
			fprintf(stderr, "Invalid paramter '%c'\n", opt);
			return 1;
		}
	}

	backend = gp_backend_init(backend_opts, 100, 100, "Backend Example");
	if (!backend) {
		fprintf(stderr, "Failed to initialize backend\n");
		return 1;
	}

	/* Handle events */
	for (;;) {
		gp_event *ev = gp_backend_ev_wait(backend);

		gp_ev_dump(ev);

		switch (ev->type) {
		case GP_EV_KEY:
			switch (ev->val) {
			case GP_KEY_ESC:
			case GP_KEY_Q:
				gp_backend_exit(backend);
				return 0;
			break;
			}
		break;
		case GP_EV_SYS:
			switch (ev->code) {
			case GP_EV_SYS_RENDER_STOP:
				gp_backend_render_stopped(backend);
			break;
			case GP_EV_SYS_RENDER_START:
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
