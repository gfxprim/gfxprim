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
 * Copyright (C) 2009-2011 Cyril Hrubis <metan@ucw.cz>                       *
 *                                                                           *
 *****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <SDL/SDL.h>

#include "GP.h"
#include "GP_SDL.h"

SDL_Surface *display = NULL;
GP_Context context, *bitmap;

int brightness = 0;

void event_loop(void)
{
	SDL_Event event;
	GP_Context *res;

	while (SDL_WaitEvent(&event) > 0) {

		switch (event.type) {

		case SDL_KEYDOWN:
			switch (event.key.keysym.sym) {
			case SDLK_ESCAPE:
				return;
			case SDLK_UP:
				brightness+=2;
			case SDLK_DOWN:
				brightness-=1;

				res = GP_FilterBrightness(bitmap, NULL, brightness, NULL);
				
				printf("brightness = %i %ux%u\n", brightness, res->w, res->h);

				GP_Blit(res, 0, 0, res->w, res->h, &context, 0, 0);
				SDL_Flip(display);
				GP_ContextFree(res);
			break;
			default:
				break;
			}
			break;
		case SDL_QUIT:
			return;
		default:
			break;
		}
	}
}

int main(int argc, char *argv[])
{
	/* Bits per pixel to be set for the display surface. */
	int display_bpp = 0;

	int i;
	for (i = 1; i < argc; i++) {
		if (strcmp(argv[i], "-16") == 0) {
			display_bpp = 16;
		}
		else if (strcmp(argv[i], "-24") == 0) {
			display_bpp = 24;
		}
		else if (strcmp(argv[i], "-32") == 0) {
			display_bpp = 32;
		}
	}

	GP_SetDebugLevel(10);

	GP_RetCode ret;

	if ((ret = GP_LoadImage(argv[1], &bitmap, NULL))) {
		fprintf(stderr, "Failed to load bitmap: %s\n", GP_RetCodeName(ret));
		return 1;
	}
	
	/* Initialize SDL */
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0) {
		fprintf(stderr, "Could not initialize SDL: %s\n", SDL_GetError());
		return 1;
	}

	/* Create a window with a software back surface */
	display = SDL_SetVideoMode(bitmap->w, bitmap->h, display_bpp, SDL_SWSURFACE);
	if (display == NULL) {
		fprintf(stderr, "Could not open display: %s\n", SDL_GetError());
		goto fail;
	}

	GP_SDL_ContextFromSurface(&context, display);

	GP_Blit(bitmap, 0, 0, bitmap->w, bitmap->h, &context, 0, 0);
	SDL_Flip(display);

	event_loop();

	SDL_Quit();
	return 0;
fail:
	SDL_Quit();
	return 1;
}

