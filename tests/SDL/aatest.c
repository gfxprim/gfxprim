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

static GP_Coord x = 10 * GP_FP_1 + GP_FP_1_2;
static GP_Coord y = 10 * GP_FP_1 + GP_FP_1_2;

SDL_UserEvent timer_event;

GP_Pixel red_pixel, green_pixel, blue_pixel, white_pixel;

static void draw(void)
{
	GP_Context *ctx = &context;

	GP_Fill(ctx, white_pixel);

	GP_Coord i;

	for (i = 0; i < 24; i++) {
	//	GP_FillRect(ctx, (x>>8),      (y + (10*i<<8) - 128)>>8,
	//	                 (x>>8) + 60, (y + ((10*i)<<8) + 64*i + 64 + 128)>>8, green_pixel);
		
		GP_FillRect_AA(ctx, x, y + ((10*i)<<8),
		                    x + (60<<8), y + ((10*i)<<8) + 64*i + 64, 0);
		
		GP_FillRect_AA(ctx, x + (80<<8), y + ((10*i)<<8) + 64,
		                    x + (140<<8), y + ((10*i)<<8) + 64*i + 128, 0);
		
		GP_FillRect_AA(ctx, x + (160<<8), y + ((10*i)<<8) + 128,
		                    x + (220<<8), y + ((10*i)<<8) + 64*i + 192, 0);
		
		GP_FillRect_AA(ctx, x + (240<<8), y + ((10*i)<<8) + 192,
		                    x + (300<<8), y + ((10*i)<<8) + 64*i + 256, 0);
		printf("%i --------------------------------------------------------\n", i);	
	}

	SDL_Flip(display);
}

void event_loop(void)
{
	SDL_Event event;

	while (SDL_WaitEvent(&event) > 0) {
		switch (event.type) {
		case SDL_KEYDOWN:
			switch (event.key.keysym.sym) {
			case SDLK_DOWN:
				y += 64;
				draw();
			break;
			case SDLK_UP:
				y -= 64;
				draw();
			break;
			case SDLK_LEFT:
				x -= 64;
				draw();
			break;
			case SDLK_RIGHT:
				x += 64;
				draw();
			break;
			case SDLK_x:
				context.x_swap = !context.x_swap;
				draw();
			break;
			case SDLK_y:
				context.y_swap = !context.y_swap;
				draw();
			break;
			case SDLK_r:
				context.axes_swap = !context.axes_swap;
				draw();
			break;
			case SDLK_ESCAPE:
			case SDLK_q:
				return;
			default:
			break;
			}
		break;
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

	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0) {
		fprintf(stderr, "Could not initialize SDL: %s\n", SDL_GetError());
		return 1;
	}

	display = SDL_SetVideoMode(320, 320, display_bpp, SDL_SWSURFACE);
	if (display == NULL) {
		fprintf(stderr, "Could not open display: %s\n", SDL_GetError());
		goto fail;
	}


	printf("Display surface properties:\n");
	printf("    width: %4d, height: %4d, pitch: %4d\n",
	       display->w, display->h, display->pitch);
	printf("    bits per pixel: %2d, bytes per pixel: %2d\n",
	       display->format->BitsPerPixel, display->format->BytesPerPixel);

	GP_SDL_ContextFromSurface(&context, display);

	red_pixel   = GP_ColorToContextPixel(GP_COL_RED, &context);
	green_pixel = GP_ColorToContextPixel(GP_COL_GREEN, &context);
	blue_pixel  = GP_ColorToContextPixel(GP_COL_BLUE, &context);
	white_pixel = GP_ColorToContextPixel(GP_COL_WHITE, &context);

	draw();

	event_loop();
	SDL_Quit();
	return 0;

fail:
	SDL_Quit();
	return 1;
}
