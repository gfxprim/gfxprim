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
 * Copyright (C) 2009-2012 Cyril Hrubis <metan@ucw.cz>                       *
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
GP_Pixel red_pixel, green_pixel, blue_pixel, white_pixel;

Uint32 timer_callback(__attribute__((unused)) Uint32 interval,
			__attribute__((unused)) void *param)
{
	timer_event.type = SDL_USEREVENT;
	SDL_PushEvent((SDL_Event *) &timer_event);
	return 0;
}

void draw_pixels(void)
{
	unsigned int i, j, k;

	GP_FillRect(&context, 0, 0, context.w, 120, white_pixel);

	unsigned int y = 20;

	for (i = 0; i <= 256; i++) {
		GP_MixPixel_Raw(&context, i + 20, y + 0, 0, i);
		GP_MixPixel_Raw(&context, i + 20, y + 1, 0, i);
		GP_MixPixel_Raw(&context, i + 20, y + 2, 0, i);
		GP_MixPixel_Raw(&context, i + 20, y + 3, 0, i);
		GP_MixPixel_Raw(&context, i + 20, y + 4, 0, i);
		
		GP_MixPixel_Raw(&context, i + 20, y + 5, red_pixel, i);
		GP_MixPixel_Raw(&context, i + 20, y + 6, red_pixel, i);
		GP_MixPixel_Raw(&context, i + 20, y + 7, red_pixel, i);
		
		GP_MixPixel_Raw(&context, i + 20, y + 8, green_pixel, i);
		GP_MixPixel_Raw(&context, i + 20, y + 9, green_pixel, i);
		GP_MixPixel_Raw(&context, i + 20, y + 10, green_pixel, i);
		
		GP_MixPixel_Raw(&context, i + 20, y + 11, blue_pixel, i);
		GP_MixPixel_Raw(&context, i + 20, y + 12, blue_pixel, i);
		GP_MixPixel_Raw(&context, i + 20, y + 13, blue_pixel, i);
		

		if (i % 16 == 0)
			GP_PutPixel(&context, i + 20, y + 14, 0);
		
		if (i % 32 == 0)
			GP_PutPixel(&context, i + 20, y + 15, 0);
		
		if (i % 64 == 0) {
			GP_PutPixel(&context, i + 20, y + 16, 0);
			GP_Print(&context, NULL, i + 20, y + 20,
			         GP_ALIGN_CENTER | GP_VALIGN_BELOW,
				 0, 0, "%u", i);
		}
		
		if (i % 128 == 0)
			GP_PutPixel(&context, i + 20, y + 17, 0);
	}

	for (i = 0; i < 7; i++)
		for (j = 0; j < 32; j++)
			for (k = 0; k < 32; k++)
				GP_MixPixel_Raw(&context, (225 * i) / 6 + j + 20,
				                y + k + 40, 0, (255 * i) / 6);

	y = 140;

	for (i = 0; i <= 256; i++) {
		GP_MixPixel_Raw(&context, i + 20, y + 0, white_pixel, i);
		GP_MixPixel_Raw(&context, i + 20, y + 1, white_pixel, i);
		GP_MixPixel_Raw(&context, i + 20, y + 2, white_pixel, i);
		GP_MixPixel_Raw(&context, i + 20, y + 3, white_pixel, i);
		GP_MixPixel_Raw(&context, i + 20, y + 4, white_pixel, i);
		
		GP_MixPixel_Raw(&context, i + 20, y + 5, red_pixel, i);
		GP_MixPixel_Raw(&context, i + 20, y + 6, red_pixel, i);
		GP_MixPixel_Raw(&context, i + 20, y + 7, red_pixel, i);
		
		GP_MixPixel_Raw(&context, i + 20, y + 8, green_pixel, i);
		GP_MixPixel_Raw(&context, i + 20, y + 9, green_pixel, i);
		GP_MixPixel_Raw(&context, i + 20, y + 10, green_pixel, i);
		
		GP_MixPixel_Raw(&context, i + 20, y + 11, blue_pixel, i);
		GP_MixPixel_Raw(&context, i + 20, y + 12, blue_pixel, i);
		GP_MixPixel_Raw(&context, i + 20, y + 13, blue_pixel, i);

		if (i % 16 == 0)
			GP_PutPixel(&context, i + 20, y + 14, white_pixel);
		
		if (i % 32 == 0)
			GP_PutPixel(&context, i + 20, y + 15, white_pixel);
		
		if (i % 64 == 0) {
			GP_PutPixel(&context, i + 20, y + 16, white_pixel);
			GP_Print(&context, NULL, i + 20, y + 20,
			         GP_ALIGN_CENTER | GP_VALIGN_BELOW,
				 white_pixel, 0, "%u", i);
		}

		if (i % 128 == 0)
			GP_PutPixel(&context, i + 20, y + 17, white_pixel);
	}
	
	for (i = 0; i < 7; i++)
		for (j = 0; j < 32; j++)
			for (k = 0; k < 32; k++)
				GP_MixPixel_Raw(&context, (225 * i) / 6 + j + 20,
				                y + k + 40, white_pixel, (255 * i) / 6);

}

void event_loop(void)
{
	SDL_Event event;

	while (SDL_WaitEvent(&event) > 0) {
		switch (event.type) {
		case SDL_USEREVENT:
			draw_pixels();
			SDL_Flip(display);
		break;
		case SDL_KEYDOWN:
		case SDL_QUIT:
			return;
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
	display = SDL_SetVideoMode(320, 240, display_bpp, SDL_SWSURFACE);
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

	/* Set up a clipping rectangle to test proper clipping of pixels */
	SDL_Rect clip_rect = {10, 10, 300, 220};
	SDL_SetClipRect(display, &clip_rect);

	GP_SDL_ContextFromSurface(&context, display);

	/* Load pixel values compatible with the display. */
	red_pixel   = GP_ColorToContextPixel(GP_COL_RED, &context);
	green_pixel = GP_ColorToContextPixel(GP_COL_GREEN, &context);
	blue_pixel  = GP_ColorToContextPixel(GP_COL_BLUE, &context);
	white_pixel = GP_ColorToContextPixel(GP_COL_WHITE, &context);

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
