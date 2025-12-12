// SPDX-License-Identifier: GPL-2.1-or-later
/*
 * Copyright (C) 2009-2026 Cyril Hrubis <metan@ucw.cz>
 */

/*

  Simple example that shows X11 window with image.

 */

#include <stdio.h>
#include <errno.h>
#include <string.h>

#include <gfxprim.h>

int main(int argc, char *argv[])
{
	gp_backend *backend;
	gp_pixmap *image;
	int x_off = 0, y_off = 0;

	if (argc != 2) {
		fprintf(stderr, "Takes image as an argument\n");
		return 1;
	}

	image = gp_load_image(argv[1], NULL);
	if (!image) {
		fprintf(stderr, "Failed to load bitmap: %s\n", strerror(errno));
		return 1;
	}

	backend = gp_x11_init(NULL, 0, 0, image->w, image->h, argv[1], 0);
	if (!backend) {
		fprintf(stderr, "Failed to initalize backend\n");
		return 1;
	}

	for (;;) {
		gp_event *ev = gp_backend_ev_wait(backend);

		switch (ev->type) {
		case GP_EV_KEY:
			if (ev->val == GP_KEY_Q) {
				gp_backend_exit(backend);
				return 0;
			}
		break;
		case GP_EV_SYS:
			switch (ev->code) {
			case GP_EV_SYS_QUIT:
				gp_backend_exit(backend);
				return 0;
			break;
			case GP_EV_SYS_RENDER_STOP:
				gp_backend_render_stopped(backend);
			break;
			case GP_EV_SYS_RENDER_RESIZE:
				x_off = ((int)ev->resize.w - (int)image->w) / 2;
				y_off = ((int)ev->resize.h - (int)image->h) / 2;
			break;
			case GP_EV_SYS_RENDER_START:
				gp_fill(backend->pixmap, 0);
				gp_blit_clipped(image, 0, 0, image->w, image->h, backend->pixmap, x_off, y_off);
				gp_backend_flip(backend);
			break;
			}
		}
	}

	return 0;
}
