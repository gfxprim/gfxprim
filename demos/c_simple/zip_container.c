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

  Simple example that loads and show image from zip container into X11 window.

 */

#include <stdio.h>
#include <errno.h>
#include <string.h>

#include <GP.h>

static GP_Backend *backend;
static GP_Context *image;
static GP_Container *container;

/*
 * Try to load next image in container, if image has different size than the
 * window request resize and blit it at the resize event in the main loop.
 *
 * Note that the resize event needs not to be granted so we really should have
 * plan b (which is omited here for the sake of simplicity).
 */
static void load_next(void)
{
	GP_ContextFree(image);

	image = GP_ContainerLoadNext(container, NULL);

	if (image == NULL)
		return;

	if (image->w != backend->context->w ||
	    image->h != backend->context->h) {
		GP_BackendResize(backend, image->w, image->h);
		return;
	}

	GP_Blit(image, 0, 0, image->w, image->h, backend->context, 0, 0);
	GP_BackendFlip(backend);
}

int main(int argc, char *argv[])
{

	if (argc != 2) {
		fprintf(stderr, "Takes path to zip or cbz as an argument\n");
		return 1;
	}

	/* Open zip container */
	container = GP_OpenZip(argv[1]);

	if (container == NULL) {
		fprintf(stderr, "Failed to open container: %s\n", strerror(errno));
		return 1;
	}

	/* Load image */
	image = GP_ContainerLoadNext(container, NULL);

	if (image == NULL) {
		fprintf(stderr, "Failed to load image %s\n", strerror(errno));
		return 1;
	}

	/* Initalize backend */
	backend = GP_BackendX11Init(NULL, 0, 0, image->w, image->h, argv[1], 0);

	if (backend == NULL) {
		fprintf(stderr, "Failed to initalize backend\n");
		return 1;
	}

	/* Blit image into the window and show it */
	GP_Blit(image, 0, 0, image->w, image->h, backend->context, 0, 0);
	GP_BackendFlip(backend);

	/* Wait for events  */
	for (;;) {
		GP_Event ev;

		GP_BackendWaitEvent(backend, &ev);

		switch (ev.type) {
		case GP_EV_KEY:
			if (!ev.code == GP_EV_KEY_DOWN)
				continue;

			switch (ev.val.val) {
			case GP_KEY_Q:
				GP_BackendExit(backend);
				return 0;
			break;
			case GP_KEY_SPACE:
				load_next();
			break;
			}
		break;
		case GP_EV_SYS:
			if (ev.code == GP_EV_SYS_RESIZE) {
				GP_BackendResizeAck(backend);
				GP_Blit(image, 0, 0, image->w, image->h,
				        backend->context, 0, 0);
				GP_BackendFlip(backend);
			}
		break;
		}
	}

	return 0;
}
