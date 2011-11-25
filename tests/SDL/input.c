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
 * Copyright (C) 2009-2011 Cyril Hrubis <metan@ucw.cz>                       *
 *                                                                           *
 *****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <SDL/SDL.h>

#include "GP.h"
#include "GP_SDL.h"

SDL_Surface *display = NULL;
GP_Context context;

SDL_TimerID timer;
SDL_UserEvent timer_event;

GP_Pixel black_pixel, red_pixel, green_pixel, white_pixel;

Uint32 timer_callback(__attribute__((unused)) Uint32 interval,
			__attribute__((unused)) void *param)
{
	timer_event.type = SDL_USEREVENT;
	SDL_PushEvent((SDL_Event *) &timer_event);
	return 30;
}

static void draw_event(GP_Event *ev)
{
	static GP_Size size = 0;

	if (ev->type != GP_EV_KEY)
		return;
	
	int align = GP_ALIGN_RIGHT|GP_VALIGN_BOTTOM;

	GP_TextClear(&context, NULL, 20, 20, align, black_pixel, size);
	size = GP_Print(&context, NULL, 20, 20, align,
	                white_pixel, black_pixel, "Key=%s",
			GP_EventKeyName(ev->val.key.key));
	SDL_Flip(display);
}

static void event_loop(void)
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
					GP_HLineXXY(&context, ev.cursor_x - 3,
					            ev.cursor_x + 3,
						    ev.cursor_y, red_pixel);
					GP_VLineXYY(&context, ev.cursor_x,
					            ev.cursor_y - 3,
						    ev.cursor_y + 3, red_pixel);
					SDL_Flip(display);
				break;
				default:
				break;
				}
			break;
			case GP_EV_REL:
				switch (ev.code) {
				static int size = 0;
				case GP_EV_REL_POS:
					if (GP_EventGetKey(&ev, GP_BTN_LEFT)) {
						GP_PutPixel(&context, ev.cursor_x,
					        	    ev.cursor_y, green_pixel);
						SDL_Flip(display);
					}
					int align = GP_ALIGN_RIGHT|GP_VALIGN_BOTTOM;

					GP_TextClear(&context, NULL, 20, 40, align,
					             black_pixel, size);
					size = GP_Print(&context, NULL, 20, 40, align,
					                white_pixel, black_pixel, "X=%3u Y=%3u",
						        ev.cursor_x, ev.cursor_y);
					SDL_Flip(display);
				break;
				}
			break;
			}
		}
	}
}

int main(int argc, char *argv[])
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

	display = SDL_SetVideoMode(480, 640, display_bpp, SDL_SWSURFACE);
	if (display == NULL) {
		fprintf(stderr, "Could not open display: %s\n", SDL_GetError());
		goto fail;
	}

	GP_EventSetScreenSize(480, 640);

	GP_SDL_ContextFromSurface(&context, display);

	red_pixel   = GP_ColorToContextPixel(GP_COL_RED, &context);
	green_pixel = GP_ColorToContextPixel(GP_COL_GREEN, &context);
	white_pixel = GP_ColorToContextPixel(GP_COL_WHITE, &context);
	black_pixel = GP_ColorToContextPixel(GP_COL_BLACK, &context);

	timer = SDL_AddTimer(30, timer_callback, NULL);
	if (timer == 0) {
		fprintf(stderr, "Could not set up timer: %s\n", SDL_GetError());
		goto fail;
	}

	event_loop();
	SDL_Quit();
	return 0;

fail:
	SDL_Quit();
	return 1;
}
