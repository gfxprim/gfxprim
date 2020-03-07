// SPDX-License-Identifier: GPL-2.1-or-later
/*
 * Copyright (C) 2009-2013 Cyril Hrubis <metan@ucw.cz>
 */

/*

  Simple example that loads and show image from zip container into X11 window.

 */

#include <stdio.h>
#include <errno.h>
#include <string.h>

#include <gfxprim.h>

static gp_backend *backend;
static gp_pixmap *image;
static gp_container *container;

/*
 * Try to load next image in container, if image has different size than the
 * window request resize and blit it at the resize event in the main loop.
 *
 * Note that the resize event needs not to be granted so we really should have
 * plan b (which is omited here for the sake of simplicity).
 */
static void load_next(void)
{
	gp_pixmap_free(image);

	image = gp_container_load_next(container, NULL);

	if (image == NULL)
		return;

	if (image->w != backend->pixmap->w ||
	    image->h != backend->pixmap->h) {
		gp_backend_resize(backend, image->w, image->h);
		return;
	}

	gp_blit_clipped(image, 0, 0, image->w, image->h, backend->pixmap, 0, 0);
	gp_backend_flip(backend);
}

int main(int argc, char *argv[])
{

	if (argc != 2) {
		fprintf(stderr, "Takes path to zip or cbz as an argument\n");
		return 1;
	}

	/* Open zip container */
	container = gp_open_zip(argv[1]);

	if (container == NULL) {
		fprintf(stderr, "Failed to open container: %s\n", strerror(errno));
		return 1;
	}

	/* Load image */
	image = gp_container_load_next(container, NULL);

	if (image == NULL) {
		fprintf(stderr, "Failed to load image %s\n", strerror(errno));
		return 1;
	}

	/* Initalize backend */
	backend = gp_x11_init(NULL, 0, 0, image->w, image->h, argv[1], 0);

	if (backend == NULL) {
		fprintf(stderr, "Failed to initalize backend\n");
		return 1;
	}

	/* Blit image into the window and show it */
	gp_blit_clipped(image, 0, 0, image->w, image->h, backend->pixmap, 0, 0);
	gp_backend_flip(backend);

	/* Wait for events  */
	for (;;) {
		gp_event ev;

		gp_backend_wait_event(backend, &ev);

		switch (ev.type) {
		case GP_EV_KEY:
			if (!(ev.code == GP_EV_KEY_DOWN))
				continue;

			switch (ev.val.val) {
			case GP_KEY_Q:
				gp_backend_exit(backend);
				return 0;
			break;
			case GP_KEY_SPACE:
				load_next();
			break;
			}
		break;
		case GP_EV_SYS:
			if (ev.code == GP_EV_SYS_RESIZE) {
				gp_backend_resize_ack(backend);
				gp_blit_clipped(image, 0, 0, image->w, image->h,
				        backend->pixmap, 0, 0);
				gp_backend_flip(backend);
			}
		break;
		}
	}

	return 0;
}
