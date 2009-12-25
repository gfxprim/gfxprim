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
#include <unistd.h>
#include <SDL/SDL.h>

#include "GP.h"

#include "../engine/TestUtils.h"

const char *TST_TestName  = "Line 01";
const char *TST_TestDesc  = "Line drawing benchmark";
const int   TST_TestCases = 1;

SDL_Surface * display = NULL;

static long colors[GP_BASIC_COLOR_COUNT];

static void do_benchmark(void)
{
	double angle;
	int x, y;
	int xcenter = display->w/2;
	int ycenter = display->h/2;

	SDL_LockSurface(display);

	GP_Clear(display, colors[GP_BLACK]);

	TST_Start("Drawing lines");
	
	for (angle = 0.0; angle < 8*M_PI; angle += 0.0001) {
		x = (int) (display->h/3 * cos(angle));
		y = (int) (display->h/3 * sin(angle));

		Uint8 r = 127.0 + 127.0 * cos(angle);
		Uint8 g = 127.0 + 127.0 * sin(angle);
		
		Uint32 color = SDL_MapRGB(display->format, r, 0, g);
	
		GP_Line(display, color, xcenter, ycenter, xcenter + x, ycenter + y);
		GP_Line(display, color, xcenter + x, ycenter + y, xcenter, ycenter);
	}
	
	TST_Stop(TST_OK | TST_STAT, NULL);

	SDL_UnlockSurface(display);
	SDL_Flip(display);
	sleep(1);
}

int main(void)
{
	TST_Init();
	
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0) {
		fprintf(stderr, "Could not initialize SDL: %s\n", SDL_GetError());
		return 1;
	}

	display = SDL_SetVideoMode(640, 480, 0, SDL_SWSURFACE);
	
	if (display == NULL) {
		fprintf(stderr, "Could not open display: %s\n", SDL_GetError());
		goto fail;
	}
	

	GP_LoadBasicColors(display, colors);
	SDL_Rect clip_rect = { 10, 10, 620, 460 };
	SDL_SetClipRect(display, &clip_rect);

	do_benchmark();

	SDL_Quit();
	TST_Exit();
fail:
	SDL_Quit();
	return 1;
}

