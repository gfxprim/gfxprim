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

/* the display */
SDL_Surface *display = NULL;
GP_Context context;

/* precomputed color pixels in display format */
static GP_Pixel black_pixel, red_pixel, yellow_pixel, green_pixel, blue_pixel,
		darkgray_pixel;

/* draw using proportional font? */
static int flag_proportional = 0;

/* center of the screen */
static int xcenter = 320;
static int ycenter = 240;

void redraw_screen(void)
{
	SDL_LockSurface(display);
	
	GP_Fill(&context, black_pixel);

	/* draw axes intersecting in the middle, where text should be shown */
	GP_HLine(&context, 0, 640, 240, darkgray_pixel);
	GP_VLine(&context, 320, 0, 480, darkgray_pixel);

	GP_TextStyle style = GP_DEFAULT_TEXT_STYLE;

	if (flag_proportional)
		style.font = &GP_default_proportional_font;
	else
		style.font = &GP_default_console_font;

	style.pixel_xspace = 4;
	style.pixel_yspace = 4;

	GP_TBoxCenteredText(&context, &style, 0, 0, 640, 480,
	         "Hello world!", darkgray_pixel);

	style.pixel_xspace = 0;
	style.pixel_yspace = 0;

	GP_TText(&context, &style, xcenter, ycenter, GP_ALIGN_LEFT|GP_VALIGN_BELOW,
	         "bottom left", yellow_pixel);
	GP_TText(&context, &style, xcenter, ycenter, GP_ALIGN_RIGHT|GP_VALIGN_BELOW,
	         "bottom right", red_pixel);
	GP_TText(&context, &style, xcenter, ycenter, GP_ALIGN_RIGHT|GP_VALIGN_ABOVE,
	         "top right", blue_pixel);
	GP_TText(&context, &style, xcenter, ycenter, GP_ALIGN_LEFT|GP_VALIGN_ABOVE,
	         "top left", green_pixel);

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
			case SDLK_SPACE:
				flag_proportional = !flag_proportional;
			break;
			case SDLK_x:
				context.x_swap = !context.x_swap;
			break;
			case SDLK_y:
				context.y_swap = !context.y_swap;
			break;
			case SDLK_r:
				context.axes_swap = !context.axes_swap;
				GP_SWAP(xcenter, ycenter);
			break;
			default:
			break;
			case SDLK_ESCAPE:
				return;
			}
			redraw_screen();
			SDL_Flip(display);
		break;

		case SDL_QUIT:
			return;
		}
	}
}

void print_instructions(void)
{
	printf("Use the following keys to control the test:\n");
	printf("    Space ............... toggle proportional/nonproportional font\n");
	printf("    X ................... mirror X\n");
	printf("    Y ................... mirror Y\n");
	printf("    R ................... reverse X and Y\n");
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

	print_instructions();

	/* Set up a clipping rectangle to test proper clipping of pixels */
	SDL_Rect clip_rect = {10, 10, 620, 460};
	SDL_SetClipRect(display, &clip_rect);

	/* Initialize a GP context from the SDL display */
	GP_SDL_ContextFromSurface(&context, display);

	/* Load colors suitable for the display */
	GP_ColorNameToPixel(&context, GP_COL_YELLOW, &yellow_pixel);
	GP_ColorNameToPixel(&context, GP_COL_BLUE, &blue_pixel);
	GP_ColorNameToPixel(&context, GP_COL_RED, &red_pixel);
	GP_ColorNameToPixel(&context, GP_COL_GREEN, &green_pixel);
	GP_ColorNameToPixel(&context, GP_COL_BLACK, &black_pixel);
	GP_ColorNameToPixel(&context, GP_COL_GRAY_DARK, &darkgray_pixel);

	redraw_screen();
	SDL_Flip(display);

	event_loop();

	SDL_Quit();
	return 0;

fail:
	SDL_Quit();
	return 1;
}

