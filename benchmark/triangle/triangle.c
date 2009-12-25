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

const char *TST_TestName  = "Triangle 01";
const char *TST_TestDesc  = "Triangle drawing benchmark";
const int   TST_TestCases = 2;

SDL_Surface *display = NULL;

static long colors[GP_BASIC_COLOR_COUNT];

static void do_benchmark(void)
{
	float angle;
	uint32_t w = display->w/2;
	uint32_t h = display->h/2;
	uint32_t l = display->h/3;

	TST_Start("Drawing triangles");
	
	for (angle = 0.0; angle < M_PI; angle += 0.001) {
		int x0, y0, x1, y1, x2, y2;

		x0 = w + l * cos(angle);
		y0 = h + l * sin(angle);
		x1 = w + l * cos(angle + 2);
		y1 = h + l * sin(angle + 2);
		x2 = w + l * cos(angle + 4);
		y2 = h + l * sin(angle + 4);

		GP_FillTriangle(display, colors[GP_RED], x0, y0, x1, y1, x2, y2);
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

