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
SDL_Surface * display = NULL;

/* Timer used for refreshing the display */
SDL_TimerID timer;

/* An event used for signaling that the timer was triggered. */
SDL_UserEvent timer_event;

/* Globally used colors. */
static long colors[GP_BASIC_COLOR_COUNT];

/* Holding flag (pauses drawing). */
static int pause_flag = 0;

Uint32 timer_callback(__attribute__((unused)) Uint32 interval,
			__attribute__((unused)) void * param)
{
	timer_event.type = SDL_USEREVENT;
	SDL_PushEvent((SDL_Event *) &timer_event);
	return 60;
}

/* Shape to draw */
#define SHAPE_FIRST	1
#define SHAPE_CIRCLE	1
#define SHAPE_ELLIPSE	2
#define SHAPE_TRIANGLE	3
#define SHAPE_RECTANGLE	4
#define SHAPE_LAST	4
static int shape = SHAPE_FIRST;

/* Draw outlines? */
static int outline_flag = 0;

/* Draw filled shapes? */
static int fill_flag = 1;

void random_point(SDL_Surface * surf, int * x, int * y)
{
	*x = random() % surf->w;
	*y = random() % surf->h;
}

void draw_random_circle(long color)
{
	int x, y;
	random_point(display, &x, &y);
	int r = random() % 50;

	if (fill_flag) {
		GP_FillCircle(display, color, x, y, r);
	}

	if (outline_flag) {
		GP_Circle(display, colors[GP_WHITE], x, y, r);
	}
}

void draw_random_ellipse(long color)
{
	int x, y;
	random_point(display, &x, &y);
	int rx = random() % 50;
	int ry = random() % 50;

	if (fill_flag) {
		GP_FillEllipse(display, color, x, y, rx, ry);
	}

	if (outline_flag) {
		GP_Ellipse(display, colors[GP_WHITE], x, y, rx, ry);
	}
}

void draw_random_triangle(long color)
{
	int x0, y0, x1, y1, x2, y2;
	random_point(display, &x0, &y0);
	random_point(display, &x1, &y1);
	random_point(display, &x2, &y2);

	if (fill_flag) {
		GP_FillTriangle(display, color, x0, y0, x1, y1, x2, y2);
	}

	if (outline_flag) {
		GP_Triangle(display, colors[GP_WHITE], x0, y0, x1, y1, x2, y2);
	}
}

void draw_random_rectangle(long color)
{
	int x0, y0, x1, y1;
	random_point(display, &x0, &y0);
	random_point(display, &x1, &y1);

	if (fill_flag) {
		GP_FillRect(display, color, x0, y0, x1, y1);
	}

	if (outline_flag) {
		GP_Rect(display, colors[GP_WHITE], x0, y0, x1, y1);
	}
}

void clear_screen(void)
{
	SDL_LockSurface(display);
	GP_Clear(display, colors[GP_BLACK]);
	SDL_UnlockSurface(display);
}

void redraw_screen(void)
{
	/* Random color. */
	long color = SDL_MapRGB(display->format,
				random() % 256,
				random() % 256,
				random() % 256);

	if (pause_flag)
		return;

	SDL_LockSurface(display);

	switch (shape) {
	case SHAPE_CIRCLE:
		draw_random_circle(color);
		break;
	
	case SHAPE_ELLIPSE:
		draw_random_ellipse(color);
		break;
	
	case SHAPE_TRIANGLE:
		draw_random_triangle(color);
		break;
	
	case SHAPE_RECTANGLE:
		draw_random_rectangle(color);
		break;
	}

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
			break;

		case SDL_KEYDOWN:
			switch (event.key.keysym.sym) {

			case SDLK_SPACE:
				shape++;
				if (shape > SHAPE_LAST)
					shape = SHAPE_FIRST;
				clear_screen();
				SDL_Flip(display);
				pause_flag = 0;
				break;

			case SDLK_p:
				pause_flag = !pause_flag;
				break;

			case SDLK_f:
				fill_flag = !fill_flag;
				if (!fill_flag && !outline_flag) {
					outline_flag = 1;
				}
				break;

			case SDLK_o:
				outline_flag = !outline_flag;
				if (!fill_flag && !outline_flag) {
					fill_flag = 1;
				}
				break;

			case SDLK_ESCAPE:
				return;

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

int main(void)
{
	/* Initialize SDL */
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0) {
		fprintf(stderr, "Could not initialize SDL: %s\n", SDL_GetError());
		return 1;
	}

	/* Create a window with a software back surface */
	display = SDL_SetVideoMode(320, 240, 0, SDL_SWSURFACE);
	if (display == NULL) {
		fprintf(stderr, "Could not open display: %s\n", SDL_GetError());
		goto fail;
	}

	/* Load pixel values for default colors */
	GP_LoadBasicColors(display, colors);

	/* Set up a clipping rectangle to test proper clipping of pixels */
	SDL_Rect clip_rect = { 10, 10, 300, 220 };
	SDL_SetClipRect(display, &clip_rect);

	/* Set up the refresh timer */
	timer = SDL_AddTimer(60, timer_callback, NULL);
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

