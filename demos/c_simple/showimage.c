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
 * Copyright (C) 2009-2012 Cyril Hrubis <metan@ucw.cz>                       *
 *                                                                           *
 *****************************************************************************/

/*
 
  Simple example that shows X11 window with image.

 */

#include <stdio.h>
#include <errno.h>
#include <string.h>

#include <GP.h>

int main(int argc, char *argv[])
{
	GP_Backend *backend;
	GP_Context *image;
	
	GP_SetDebugLevel(10);

	if (argc != 2) {
		fprintf(stderr, "Takes image as an argument\n");
		return 1;
	}

	/* Load image */
	image = GP_LoadImage(argv[1], NULL);

	if (image == NULL) {
		fprintf(stderr, "Failed to load bitmap: %s\n", strerror(errno));
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
		GP_BackendWait(backend);
	
		GP_Event ev;

		while (GP_EventGet(&ev)) {
			if (ev.type == GP_EV_KEY && ev.val.val == GP_KEY_Q) {
				GP_BackendExit(backend);
				return 0;
			}
		}
	}

	return 0;
}
