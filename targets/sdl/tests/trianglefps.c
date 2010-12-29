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

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <SDL/SDL.h>

#include "GP.h"
#include "GP_SDL.h"

/* Draw filled triangles? */
int filled = 0;

/* The surface used as a display (in fact it is a software surface). */
SDL_Surface *display = NULL;
GP_Context context;

/* Frames per second. */
int fps = 0, fps_min = 1000000, fps_max = 0;

/* Color pixel values in display format. */
GP_Pixel black, white;

/*
 * Timer used for FPS measurement and key reactions.
 * SDL_USEREVENT is triggered each second.
 */

SDL_TimerID timer;

SDL_UserEvent timer_event;

Uint32 timer_callback(__attribute__((unused)) Uint32 interval,
			__attribute__((unused)) void * param)
{
	timer_event.type = SDL_USEREVENT;
	SDL_PushEvent((SDL_Event *) &timer_event);
	return 1000;
}


void draw_frame(void)
{
	int x0 = 0;
	int y0 = random() % display->h;
	int x1 = display->w;
	int y1 = random() % display->h;
	int x2 = display->w/2;
	int y2 = random() % display->h;
	GP_Color color = GP_RGB888_PACK(random() % 255, random() % 255, random() % 255);

	GP_Pixel pixel;
	GP_ColorToPixel(&context, color, &pixel);
	
	if (filled) {
		GP_FillTriangle(&context, x0, y0, x1, y1, x2, y2, pixel);
	} else {
		GP_Triangle(&context, x0, y0, x1, y1, x2, y2, pixel);
	}
}

void event_loop(void)
{
	SDL_Event event;

	for (;;) {
        	while (SDL_PollEvent(&event) > 0) {
			switch (event.type) {
				case SDL_USEREVENT:
					SDL_Flip(display);
					fprintf(stdout, "%d triangles/second, min = %d, max = %d\r", fps, fps_min, fps_max);
					fflush(stdout);

					/* Update frames per second */
					if (fps < fps_min)
						fps_min = fps;
					if (fps > fps_max)
						fps_max = fps;
					fps = 0;
            				break;
				case SDL_KEYDOWN:
				case SDL_QUIT:
					return;
			}
		}
		draw_frame();
		fps++;
	}
}

int main(int argc, char ** argv)
{
	int bit_depth = 0;

	int i;
	for (i = 1; i < argc; i++) {
		if (strcmp(argv[i], "-f") == 0) {
			filled = 1;
		}
		else if (strcmp(argv[i], "-16") == 0) {
			bit_depth = 16;
		}
		else if (strcmp(argv[i], "-24") == 0) {
			bit_depth = 24;
		}
		else if (strcmp(argv[i], "-32") == 0) {
			bit_depth = 32;
		}
	}

	/* Initialize SDL */
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0) {
		fprintf(stderr, "Could not initialize SDL: %s\n", SDL_GetError());
		return 1;
	}

	/* Create a window with a software back surface */
	display = SDL_SetVideoMode(640, 480, bit_depth, SDL_SWSURFACE);
	if (display == NULL) {
		fprintf(stderr, "Could not open display: %s\n", SDL_GetError());
		goto fail;
	}

	/* Print basic information about the surface */
	printf("Display surface properties:\n");
	printf("    width: %4d, height: %4d, pitch: %4d\n",
	       display->w, display->h, display->pitch);
	printf("    bits per pixel: %2d, bytes per pixel: %2d\n",
	       display->format->BitsPerPixel, display->format->BytesPerPixel);
	printf("Machine properties:\n");
	printf("    sizeof(int) = %u, sizeof(long) = %u\n",
	       (unsigned int)sizeof(int), (unsigned int)sizeof(long));

	/* Set up a clipping rectangle to test proper clipping of pixels */
	SDL_Rect clip_rect = { 10, 10, 620, 460 };
	SDL_SetClipRect(display, &clip_rect);

	GP_SDL_ContextFromSurface(&context, display);

	GP_ColorNameToPixel(&context, GP_COL_WHITE, &white);
	GP_ColorNameToPixel(&context, GP_COL_BLACK, &black);

	/* Set up the timer */
	timer = SDL_AddTimer(1000, timer_callback, NULL);
	if (timer == 0) {
		fprintf(stderr, "Could not set up timer: %s\n", SDL_GetError());
		goto fail;
	}

	/* Enter the event loop */
	event_loop();

	/* Preserve the last result by a newline */
	fprintf(stdout, "\n");

	/* We're done */
	SDL_Quit();
	return 0;

fail:
	SDL_Quit();
	return 1;
}

