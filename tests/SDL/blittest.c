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

static GP_Pixel black;
static GP_Pixel white;

SDL_Surface *display = NULL;
GP_Context context;
GP_Context *bitmap, *bitmap_raw, *bitmap_conv;

SDL_TimerID timer;

SDL_UserEvent timer_event;

static int pause_flag = 0;

static int bitmap_x, bitmap_y, bitmap_vx = -3, bitmap_vy = -3;

Uint32 timer_callback(__attribute__((unused)) Uint32 interval,
			__attribute__((unused)) void *param)
{
	timer_event.type = SDL_USEREVENT;
	SDL_PushEvent((SDL_Event *) &timer_event);
	return 10;
}

static char text_buf[255];

void redraw_screen(void)
{
	if (pause_flag)
		return;

	bitmap_x += bitmap_vx;
	bitmap_y += bitmap_vy;

	if (bitmap_x + bitmap->w > context.w) {
		bitmap_vx = -bitmap_vx;
		bitmap_x += bitmap_vx;
	}

	if (bitmap_x < 0) {
		bitmap_vx = -bitmap_vx;
		bitmap_x += bitmap_vx;
	}

	if (bitmap_y + bitmap->h > context.h) {
		bitmap_vy = -bitmap_vy;
		bitmap_y += bitmap_vy;
	}
	
	if (bitmap_y < 0) {
		bitmap_vy = -bitmap_vy;
		bitmap_y += bitmap_vy;
	}

	SDL_LockSurface(display);

	GP_Text(&context, NULL, 20, 20, GP_ALIGN_RIGHT|GP_VALIGN_BOTTOM,
	        white, black, text_buf);
	GP_Blit(bitmap, 0, 0, bitmap->w, bitmap->h, &context, bitmap_x, bitmap_y);

	SDL_UpdateRect(display, bitmap_x, bitmap_y, bitmap->w, bitmap->h);
	SDL_UpdateRect(display, 20, 20, 300, 50);

	SDL_UnlockSurface(display);
}

static void change_bitmap(void)
{
	if (bitmap == bitmap_raw)
		bitmap = bitmap_conv;
	else
		bitmap = bitmap_raw;
	
	snprintf(text_buf, sizeof(text_buf), "Blitting '%s' -> '%s'",
	         GP_PixelTypes[bitmap->pixel_type].name,
		 GP_PixelTypes[context.pixel_type].name);

	GP_FillRectXYWH(&context, 20, 20, 300, 50, black);
}

void event_loop(void)
{
	SDL_Event event;

	while (SDL_WaitEvent(&event) > 0) {

		switch (event.type) {

		case SDL_USEREVENT:
			redraw_screen();
			break;
		case SDL_KEYDOWN:
			switch (event.key.keysym.sym) {
			case SDLK_p:
				pause_flag = !pause_flag;
			break;
			case SDLK_SPACE:
				change_bitmap();
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

	if ((ret = GP_LoadPPM("ball.ppm", &bitmap_raw))) {
		fprintf(stderr, "Failed to load bitmap: %s\n", GP_RetCodeName(ret));
		return 1;
	}
	
	/* Initialize SDL */
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0) {
		fprintf(stderr, "Could not initialize SDL: %s\n", SDL_GetError());
		return 1;
	}

	/* Create a window with a software back surface */
	display = SDL_SetVideoMode(640, 480, display_bpp, SDL_SWSURFACE);
	if (display == NULL) {
		fprintf(stderr, "Could not open display: %s\n", SDL_GetError());
		goto fail;
	}

	GP_SDL_ContextFromSurface(&context, display);
	
	bitmap_conv = GP_ContextConvert(bitmap_raw, context.pixel_type);
	change_bitmap();

	black = GP_ColorToContextPixel(GP_COL_BLACK, &context);
	white = GP_ColorToContextPixel(GP_COL_WHITE, &context);

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

