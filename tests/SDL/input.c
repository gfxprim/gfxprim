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
#include "GP_SDL.h"

/* The surface used as a display (in fact it is a software surface). */
SDL_Surface *display = NULL;
GP_Context context;

/* Timer used for refreshing the display */
SDL_TimerID timer;

/* An event used for signaling that the timer was triggered. */
SDL_UserEvent timer_event;

/* Values for color pixels in display format. */
GP_Pixel black_pixel, red_pixel, green_pixel, blue_pixel, white_pixel;

Uint32 timer_callback(__attribute__((unused)) Uint32 interval,
			__attribute__((unused)) void *param)
{
	timer_event.type = SDL_USEREVENT;
	SDL_PushEvent((SDL_Event *) &timer_event);
	return 30;
}

void draw_pixel(void)
{
//	GP_Color pixel, conv;
	int x = random() % 320;
	int y = random() % 240;


	GP_PutPixel(&context, x, y, green_pixel);
}

void draw_event(GP_Event *ev)
{
	if (ev->type != GP_EV_KEY)
		return;

	GP_FillRect(&context, 0, 0, 200, 20, black_pixel);
	GP_Text(&context, NULL, 0, 0, GP_ALIGN_RIGHT|GP_VALIGN_BOTTOM,
	        GP_EventKeyName(ev->val.key.key), white_pixel);
	SDL_Flip(display);
}

void draw_pixels(void)
{
	SDL_LockSurface(display);

	/* Draw some pixels (exact number is not important). */
	int i;
	for (i = 0; i < 30; i++)
		draw_pixel();

	SDL_UnlockSurface(display);
}

void event_loop(void)
{
	SDL_Event event;

	while (SDL_WaitEvent(&event) > 0) {
		GP_InputDriverSDLEventPut(&event);
	
		while (GP_EventQueued()) {
			GP_Event ev;

			GP_EventGet(&ev);
			GP_EventDump(&ev);

			switch (ev.type) {
			case GP_EV_KEY:
				draw_event(&ev);

				switch (ev.val.key.key) {
				case GP_KEY_ESC:
					SDL_Quit();
					exit(0);
				break;
				case GP_BTN_LEFT:
					GP_PutPixel(&context, ev.cursor_x,
					            ev.cursor_y, red_pixel);
					SDL_Flip(display);
				break;
				default:
				break;
				}
			break;
			case GP_EV_REL:
				switch (ev.code) {
				case GP_EV_REL_POS:
					if (GP_EventGetKey(&ev, GP_BTN_LEFT)) {
						GP_PutPixel(&context, ev.cursor_x,
					        	    ev.cursor_y, green_pixel);
						SDL_Flip(display);
					}
				break;
				}
			break;
			}
		}
	}
}

int main(int argc, char **argv)
{
	int display_bpp = 0;

	int i;
	for (i = 1; i < argc; i++) {
		if (strcmp(argv[i], "-16") == 0) {
			display_bpp = 16;
		} else if (strcmp(argv[i], "-24") == 0) {
			display_bpp = 24;
		}
	}

	/* Initialize SDL */
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0) {
		fprintf(stderr, "Could not initialize SDL: %s\n", SDL_GetError());
		return 1;
	}

	/* Create a window with a software back surface */
	display = SDL_SetVideoMode(480, 640, display_bpp, SDL_SWSURFACE);
	if (display == NULL) {
		fprintf(stderr, "Could not open display: %s\n", SDL_GetError());
		goto fail;
	}

	GP_EventSetScreenSize(480, 640);

	/* Print basic information about the surface */
	printf("Display surface properties:\n");
	printf("    width: %4d, height: %4d, pitch: %4d\n",
	       display->w, display->h, display->pitch);
	printf("    bits per pixel: %2d, bytes per pixel: %2d\n",
	       display->format->BitsPerPixel, display->format->BytesPerPixel);

	/* Set up a clipping rectangle to test proper clipping of pixels */
	SDL_Rect clip_rect = {10, 10, 300, 220};
	SDL_SetClipRect(display, &clip_rect);

	GP_SDL_ContextFromSurface(&context, display);

	/* Load pixel values compatible with the display. */
	red_pixel   = GP_ColorToPixel(&context, GP_COL_RED);
	green_pixel = GP_ColorToPixel(&context, GP_COL_GREEN);
	blue_pixel  = GP_ColorToPixel(&context, GP_COL_BLUE);
	white_pixel = GP_ColorToPixel(&context, GP_COL_WHITE);
	black_pixel = GP_ColorToPixel(&context, GP_COL_BLACK);

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
