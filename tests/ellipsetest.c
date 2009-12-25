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

#include <stdio.h>
#include <stdlib.h>
#include <SDL/SDL.h>

#include "GP.h"

/* The surface used as a display (in fact it is a software surface). */
SDL_Surface *display = NULL;

/* Timer used for refreshing the display */
SDL_TimerID timer;

/* An event used for signaling that the timer was triggered. */
SDL_UserEvent timer_event;

Uint32 timer_callback(__attribute__((unused)) Uint32 interval,
			__attribute__((unused)) void * param)
{
	timer_event.type = SDL_USEREVENT;
	SDL_PushEvent((SDL_Event *) &timer_event);
	return 60;
}

/* Orientation flag (horizontal or vertical). */
int orientation_flag = 0;

/* Fill flag. */
int fill_flag = 0;

/* Radius ratio. */
int rratio = 2;

/* True if filling is drawn first, false if shapes are drawn first. */
int fill_first = 0;

/* True to draw axes. */
int draw_axes = 0;

/* Minimum radius. */
int min_radius = 3;

/* Basic colors in display format. */
static long colors[GP_BASIC_COLOR_COUNT];

void draw_pixels(void)
{
	int rx, ry, active_rx, active_ry;
	int intensity;
	long fill_color;

	/* Set up a clipping rectangle to test proper clipping of pixels */
	SDL_Rect clip_rect = { 20, 20, 600, 440 };
	SDL_SetClipRect(display, &clip_rect);

	SDL_LockSurface(display);

	/* Clear screen */
	GP_Clear(display, colors[GP_BLACK]);

	for (rx = 210 + min_radius; rx >= min_radius; rx -= 7) {
		ry = rratio*rx;
		intensity = (rx % 4) * 50;
		fill_color = SDL_MapRGB(display->format, intensity, intensity, intensity);

		if (orientation_flag) {
			active_rx = ry;
			active_ry = rx;
		}
		else {
			active_rx = rx;
			active_ry = ry;
		}

		if (fill_flag) {
			if (fill_first) {
				GP_FillEllipse(display, fill_color, 320, 240, active_rx, active_ry);
				GP_Ellipse(display, colors[GP_WHITE], 320, 240, active_rx, active_ry);
			}
			else {
				GP_Ellipse(display, colors[GP_WHITE], 320, 240, active_rx, active_ry);
				GP_FillEllipse(display, fill_color, 320, 240, active_rx, active_ry);
			}
		}
		else {
			GP_Ellipse(display, colors[GP_WHITE], 320, 240, active_rx, active_ry);
		}
	}

	if (draw_axes) {

		if (orientation_flag) {
			active_rx = rratio * min_radius;
			active_ry = min_radius;
		} else {
			active_rx = min_radius;
			active_ry = rratio*min_radius;
		}

		GP_Line(display, colors[GP_RED], 0, 240, 640, 240);
		GP_Line(display, colors[GP_RED], 320, 0, 320, 640);
		GP_Line(display, colors[GP_GREEN], 0, 240+active_ry, 640, 240+active_ry);
		GP_Line(display, colors[GP_GREEN], 0, 240-active_ry, 640, 240-active_ry);
		GP_Line(display, colors[GP_GREEN], 320+active_rx, 0, 320+active_rx, 640);
		GP_Line(display, colors[GP_GREEN], 320-active_rx, 0, 320-active_rx, 640);
	}

	SDL_UnlockSurface(display);
}

void event_loop(void)
{
	SDL_Event event;

	/* Initial redraw */
	draw_pixels();
	SDL_Flip(display);

	while (SDL_WaitEvent(&event) > 0) {

		switch (event.type) {

#if 0
			case SDL_USEREVENT:
				draw_pixels();
				SDL_Flip(display);
				break;
#endif

			case SDL_VIDEOEXPOSE:
				draw_pixels();
				SDL_Flip(display);
				break;

			case SDL_KEYDOWN:
				if (event.key.keysym.sym == SDLK_ESCAPE) {
					return;
				}
				else if (event.key.keysym.sym == SDLK_o) {
					orientation_flag = !orientation_flag;
				}
				else if (event.key.keysym.sym == SDLK_f) {
					fill_flag = !fill_flag;
				}
				else if (event.key.keysym.sym == SDLK_1) {
					rratio = 1;
				}
				else if (event.key.keysym.sym == SDLK_2) {
					rratio = 2;
				}
				else if (event.key.keysym.sym == SDLK_3) {
					rratio = 3;
				}
				else if (event.key.keysym.sym == SDLK_4) {
					rratio = 4;
				}
				else if (event.key.keysym.sym == SDLK_m) {
					fill_first = !fill_first;
				}
				else if (event.key.keysym.sym == SDLK_r) {
					min_radius++;
					if (min_radius > 10)
						min_radius = 3;
				}
				else if (event.key.keysym.sym == SDLK_x) {
					draw_axes = !draw_axes;
				}

				draw_pixels();
				SDL_Flip(display);
				break;

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

#if 0
	/* Set up the refresh timer */
	timer = SDL_AddTimer(60, timer_callback, NULL);
	if (timer == 0) {
		fprintf(stderr, "Could not set up timer: %s\n", SDL_GetError());
		goto fail;
	}
#endif

	GP_LoadBasicColors(display, colors);

	/* Enter the event loop */
	event_loop();

	/* We're done */
	SDL_Quit();
	return 0;

fail:
	SDL_Quit();
	return 1;
}

