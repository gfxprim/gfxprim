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

/* The surface used as a display (in fact it is a software surface). */
SDL_Surface *display = NULL;
GP_Context context;

/* Timer used for refreshing the display */
SDL_TimerID timer;

/* An event used for signaling that the timer was triggered. */
SDL_UserEvent timer_event;

/* Values for color pixels in display format. */
long black;
long white;

Uint32 timer_callback(__attribute__((unused)) Uint32 interval,
			__attribute__((unused)) void * param)
{
	timer_event.type = SDL_USEREVENT;
	SDL_PushEvent((SDL_Event *) &timer_event);
	return 30;
}

double start_angle = 0.0;

void redraw_screen(void)
{
	double angle;
	int x, y;
	int xcenter = display->w/2;
	int ycenter = display->h/2;

	SDL_LockSurface(display);
	SDL_FillRect(display, NULL, black);

	for (angle = 0.0; angle < 2*M_PI; angle += 0.1) {
		x = (int) (display->w/2 * cos(start_angle + angle));
		y = (int) (display->h/2 * sin(start_angle + angle));

		Uint8 r = 127.0 + 127.0 * cos(start_angle + angle);
		Uint8 g = 127.0 + 127.0 * sin(start_angle + angle);
		
		Uint32 color = SDL_MapRGB(display->format, r, 0, g);
	
		/*
		 * Draw the line forth and back to detect any pixel change
		 * between one direction and the other.
		 */
		GP_Line(&context, xcenter, ycenter, xcenter + x, ycenter + y, color);
		GP_Line(&context, xcenter + x, ycenter + y, xcenter, ycenter, color);
	}

	/* axes */
	GP_HLine(&context, 0, display->w, ycenter, white);
	GP_VLine(&context, xcenter, 0, display->h, white);

	SDL_UnlockSurface(display);
}

void event_loop(void)
{
	SDL_Event event;

        while (SDL_WaitEvent(&event) > 0) {
		switch (event.type) {
			case SDL_USEREVENT:
                		redraw_screen();
				SDL_Flip(display);
				start_angle += 0.01;
				if (start_angle > 2*M_PI) {
					start_angle = 0.0;
				}
            		break;
			case SDL_KEYDOWN:
			case SDL_QUIT:
				return;
		}
	}
}

int main(void)
{
	/* Initialize SDL */
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0) {
		fprintf(stderr, "Could not initialize SDL: %s\n", SDL_GetError());
		return 1;
	}

	/* Create a window with a software back surface */
	display = SDL_SetVideoMode(640, 480, 0, SDL_SWSURFACE);
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

	/* Get colors */
	black = SDL_MapRGB(display->format, 0, 0, 0);
	white = SDL_MapRGB(display->format, 255, 255, 255);

	/* Set up a clipping rectangle to test proper clipping of pixels */
	SDL_Rect clip_rect = { 10, 10, 620, 460 };
	SDL_SetClipRect(display, &clip_rect);

	GP_SDL_ContextFromSurface(display, &context);

	/* Set up the refresh timer */
	timer = SDL_AddTimer(30, timer_callback, NULL);
	if (timer == 0) {
		fprintf(stderr, "Could not set up timer: %s\n", SDL_GetError());
		goto fail;
	}

	/* Enter the event loop */
	event_loop();

	/* We're done */
	SDL_Quit();
	return 0;

fail:
	SDL_Quit();
	return 1;
}

