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
 * Copyright (C) 2009-2010 Cyril Hrubis <metan@ucw.cz>                       *
 *                                                                           *
 *****************************************************************************/

#include "GP.h"
#include "GP_SDL.h"

#include <stdio.h>
#include <string.h>

GP_RetCode GP_SDL_VideoInit(GP_Context *context, int width, int height,
	int argc, char **argv)
{
	if (context == NULL) {
		return GP_ENULLPTR;
	}

	/* switches that can be set on the command line */
	int display_bpp = 0;
	int debug = 0;

	if (argc > 0) {

		if (argv == NULL) {
			return GP_ENULLPTR;
		}

		/* extract settings from the command line */
		int i;
		for (i = 1; i < argc; i++) {
			if (strcmp(argv[i], "--8bpp") == 0) {
				display_bpp = 8;
			}
			else if (strcmp(argv[i], "--16bpp") == 0) {
				display_bpp = 16;
			}
			else if (strcmp(argv[i], "--24bpp") == 0) {
				display_bpp = 24;
			}
			else if (strcmp(argv[i], "--32bpp") == 0) {
				display_bpp = 32;
			}
			else if (strcmp(argv[i], "--debug") == 0) {
				debug = 1;
			}
		}
	}

	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0) {
		if (debug) {
			fprintf(stderr, "Error: Could not initialize SDL: %s\n",
				SDL_GetError());
		}
		return GP_EBACKENDLOST;
	}

	SDL_Surface *display = NULL;
	display = SDL_SetVideoMode(width, height, display_bpp, SDL_SWSURFACE);
	if (display == NULL) {
		if (debug) {
			fprintf(stderr, "Error: Unable to set video mode: %s\n",
				SDL_GetError());
		}
		SDL_Quit();
		return GP_EINVAL;
	}

	if (debug) {
		printf("Display properties:\n");
		printf("    width: %4d, height: %4d, pitch: %4d\n",
			display->w, display->h, display->pitch);
		printf("    bits per pixel: %2d, bytes per pixel: %2d\n",
			display->format->BitsPerPixel, display->format->BytesPerPixel);
		printf("    pixel bit masks: R=%x, G=%x, B=%x, A=%x\n",
			display->format->Rmask, display->format->Gmask,
			display->format->Bmask, display->format->Amask);
	}

	GP_RetCode retcode;
	retcode = GP_SDL_ContextFromSurface(context, display);
	if (retcode != GP_ESUCCESS) {
		SDL_Quit();
		return retcode;
	}

	return GP_ESUCCESS;
}
