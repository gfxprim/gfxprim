// SPDX-License-Identifier: GPL-2.1-or-later
/*
 * Copyright (C) 2009-2010 Jiri "BlueBear" Dluhos
 *                         <jiri.bluebear.dluhos@gmail.com>
 *
 * Copyright (C) 2009-2013 Cyril Hrubis <metan@ucw.cz>
 */

 /*

   This example shows how to mix SDL with GFXprim.

  */

#include <stdio.h>
#include <stdlib.h>
#include <SDL/SDL.h>
#include <gfxprim.h>
#include <backends/gp_sdl_pixmap.h>

#define W 320
#define H 240

static SDL_Surface *display = NULL;
static gp_pixmap pixmap;

static gp_pixel black_pixel, darkgray_pixel;

void redraw_screen(void)
{
	SDL_LockSurface(display);

	gp_fill(&pixmap, black_pixel);

	gp_text(&pixmap, NULL, W/2, 20, GP_ALIGN_CENTER | GP_VALIGN_BELOW,
	        darkgray_pixel, black_pixel, "GFXprim SDL Demo");

	gp_line(&pixmap, 0, 0, W-1, H-1, darkgray_pixel);
	gp_line(&pixmap, 0, H-1, W-1, 0, darkgray_pixel);

	SDL_UnlockSurface(display);
}

void event_loop(void)
{
	SDL_Event event;

	while (SDL_WaitEvent(&event) > 0) {
		switch (event.type) {

		case SDL_VIDEOEXPOSE:
			redraw_screen();
			SDL_Flip(display);
		break;

		case SDL_KEYDOWN:
			switch (event.key.keysym.sym) {
			case SDLK_ESCAPE:
				return;
			default:
			break;
			}
		break;

		case SDL_QUIT:
			return;
		}
	}
}

int main(void)
{
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0) {
		fprintf(stderr, "Could not initialize SDL: %s\n", SDL_GetError());
		return 1;
	}

	display = SDL_SetVideoMode(W, H, 0, SDL_SWSURFACE);

	if (display == NULL) {
		fprintf(stderr, "Could not open display: %s\n", SDL_GetError());
		SDL_Quit();
		return 1;
	}

	gp_pixmap_from_sdl_surface(&pixmap, display);

	black_pixel     = gp_rgb_to_pixmap_pixel(0x00, 0x00, 0x00, &pixmap);
	darkgray_pixel  = gp_rgb_to_pixmap_pixel(0x7f, 0x7f, 0x7f, &pixmap);

	redraw_screen();
	SDL_Flip(display);

	event_loop();

	SDL_Quit();
	return 0;
}

