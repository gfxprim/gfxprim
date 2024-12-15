// SPDX-License-Identifier: GPL-2.1-or-later
/*
 * Copyright (C) 2009-2013 Cyril Hrubis <metan@ucw.cz>
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

	if (argc != 2) {
		fprintf(stderr, "Takes image as an argument\n");
		return 1;
	}

	/* Load image */
	image = gp_load_image(argv[1], NULL);

	if (!image) {
		fprintf(stderr, "Failed to load bitmap: %s\n", strerror(errno));
		return 1;
	}

	/* Initalize backend */
	backend = gp_x11_init(NULL, 0, 0, image->w, image->h, argv[1], 0);

	if (!backend) {
		fprintf(stderr, "Failed to initalize backend\n");
		return 1;
	}

	/* Blit image into the window and show it */
	gp_blit(image, 0, 0, image->w, image->h, backend->pixmap, 0, 0);
	gp_backend_flip(backend);

	/* Wait for events  */
	for (;;) {
		gp_event *ev = gp_backend_ev_wait(backend);

		if (ev->type == GP_EV_KEY && ev->val == GP_KEY_Q) {
			gp_backend_exit(backend);
			return 0;
		}

		if (ev->type == GP_EV_SYS && ev->code == GP_EV_SYS_RESIZE) {
			int cx, cy;

			gp_backend_resize_ack(backend);

			cx = ((int)backend->pixmap->w - (int)image->w) / 2;
			cy = ((int)backend->pixmap->h - (int)image->h) / 2;

			gp_fill(backend->pixmap, 0);
			gp_blit_clipped(image, 0, 0, image->w, image->h, backend->pixmap, cx, cy);
			gp_backend_flip(backend);
		}
	}

	return 0;
}
