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
 * Copyright (C) 2009-2012 Cyril Hrubis <metan@ucw.cz>                       *
 *                                                                           *
 *****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <SDL/SDL.h>

#include "GP.h"
#include "GP_SDL.h"

SDL_Surface *display = NULL;
GP_Context context;

static GP_Pixel black_pixel, red_pixel, yellow_pixel, green_pixel, blue_pixel,
		darkgray_pixel;

static int font_flag = 0;

static int X = 640;
static int Y = 480;

GP_FontFace *font = NULL;

void redraw_screen(void)
{
	SDL_LockSurface(display);
	
	GP_Fill(&context, black_pixel);

	/* draw axes intersecting in the middle, where text should be shown */
	GP_HLine(&context, 0, X, Y/2, darkgray_pixel);
	GP_VLine(&context, X/2, 0, Y, darkgray_pixel);

	GP_TextStyle style = GP_DEFAULT_TEXT_STYLE;

	switch (font_flag) {
	case 0:
		style.font = &GP_DefaultProportionalFont;
	break;
	case 1:
		style.font = &GP_DefaultConsoleFont;
	break;
	case 2:
		style.font = font;
	break;
	}

	GP_Text(&context, &style, X/2, Y/2, GP_ALIGN_LEFT|GP_VALIGN_BELOW,
	        yellow_pixel, black_pixel, "bottom left");
	GP_Text(&context, &style, X/2, Y/2, GP_ALIGN_RIGHT|GP_VALIGN_BELOW,
	        red_pixel, black_pixel, "bottom right");
	GP_Text(&context, &style, X/2, Y/2, GP_ALIGN_RIGHT|GP_VALIGN_ABOVE,
	        blue_pixel, black_pixel, "top right");
	GP_Text(&context, &style, X/2, Y/2, GP_ALIGN_LEFT|GP_VALIGN_ABOVE,
	        green_pixel, black_pixel, "top left");

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
				font_flag += 1;

				if (font) {
					if (font_flag >= 3)
						font_flag = 0;
				} else {
					if (font_flag >= 2)
						font_flag = 0;
				}
			break;
			case SDLK_x:
				context.x_swap = !context.x_swap;
			break;
			case SDLK_y:
				context.y_swap = !context.y_swap;
			break;
			case SDLK_r:
				context.axes_swap = !context.axes_swap;
				GP_SWAP(X, Y);
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

int main(int argc, char *argv[])
{
	GP_SetDebugLevel(10);

	if (argc > 1)
		font = GP_FontFaceLoad(argv[1], 0, 16);
	
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0) {
		fprintf(stderr, "Could not initialize SDL: %s\n", SDL_GetError());
		return 1;
	}

	display = SDL_SetVideoMode(X, Y, 0, SDL_SWSURFACE);
	if (display == NULL) {
		fprintf(stderr, "Could not open display: %s\n", SDL_GetError());
		goto fail;
	}

	print_instructions();

	SDL_Rect clip_rect = {10, 10, X-10, Y-10};
	SDL_SetClipRect(display, &clip_rect);

	GP_SDL_ContextFromSurface(&context, display);

	black_pixel     = GP_ColorToContextPixel(GP_COL_BLACK, &context);
	red_pixel       = GP_ColorToContextPixel(GP_COL_RED, &context);
	blue_pixel      = GP_ColorToContextPixel(GP_COL_BLUE, &context);
	green_pixel     = GP_ColorToContextPixel(GP_COL_GREEN, &context);
	yellow_pixel    = GP_ColorToContextPixel(GP_COL_YELLOW, &context);
	darkgray_pixel  = GP_ColorToContextPixel(GP_COL_GRAY_DARK, &context);

	redraw_screen();
	SDL_Flip(display);

	event_loop();

	SDL_Quit();
	return 0;

fail:
	SDL_Quit();
	return 1;
}

