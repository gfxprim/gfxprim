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

/*
  
  Simple test for triangle drawing runtime.

 */

#include "GP_SDL.h"

#include <SDL/SDL.h>
#include <math.h>

#define TIMER_TICK 50
#define DISPLAY_W 640
#define DISPLAY_H 480
#define sqr(x) ((x)*(x))
#define sgn(x) ((x)>0 ? 1 : -1)

SDL_Surface *display;
GP_Context context;

SDL_TimerID timer;

int iter, l, way = 1;
GP_Color black = GP_COLNAME_PACK(GP_COL_BLACK);
GP_Color blue  = GP_COLNAME_PACK(GP_COL_BLUE);
GP_Color gray  = GP_COLNAME_PACK(GP_COL_GRAY_LIGHT);

static void sierpinsky(SDL_Surface *surf, GP_Color color, float x1, float y1, float x4, float y4, int iter)
{
	float x2, y2, x3, y3, x5, y5;
	
	if (iter <= 0) {
		GP_Line(&context, x1, y1, x4, y4, black);
		return;
	}

	x2 = floor((2*x1 + x4)/3);
	y2 = floor((2*y1 + y4)/3);

	x3 = floor((2*x4 + x1)/3);
	y3 = floor((2*y4 + y1)/3);

	x5 = (x1+x4)/2 + (y2 - y3)*sqrt(3.00/4);
	y5 = (y1+y4)/2 + (x3 - x2)*sqrt(3.00/4);

	GP_FillTriangle(&context, x2, y2, x3, y3, x5, y5, color);

	sierpinsky(surf, color, x1, y1, x2, y2, iter - 1);
	sierpinsky(surf, color, x2, y2, x5, y5, iter - 1);
	sierpinsky(surf, color, x5, y5, x3, y3, iter - 1);
	sierpinsky(surf, color, x3, y3, x4, y4, iter - 1);
}

static void draw(SDL_Surface *surf, int x, int y, int l, int iter)
{
	float x1, y1, x2, y2, x3, y3;
	int w = surf->w;
	int h = surf->h;

	l = ((w < h ? w : h) - 20)/(5 - 1.00*iter/120);
	
	x1 = sin(1.00 * iter/57) * l + x;
	y1 = cos(1.00 * iter/57) * l + y;
	
	x2 = sin(1.00 * (iter+120)/57) * l + x;
	y2 = cos(1.00 * (iter+120)/57) * l + y;
	
	x3 = sin(1.00 * (iter+240)/57) * l + x;
	y3 = cos(1.00 * (iter+240)/57) * l + y;

	GP_FillRect(&context, 0, 0, context.columns, context.rows, gray);

	GP_FillTriangle(&context, x1, y1, x2, y2, x3, y3, blue);

	sierpinsky(surf, blue, x1, y1, x2, y2, iter/60%6);
	sierpinsky(surf, blue, x2, y2, x3, y3, iter/60%6);
	sierpinsky(surf, blue, x3, y3, x1, y1, iter/60%6);
	
	SDL_UpdateRect(surf, 0, 0, surf->w, surf->h);
}

Uint32 timer_callback(Uint32 interval __attribute__ ((unused)), void *ptr __attribute__ ((unused)))
{
	iter += 2 * way;
	
	if (iter + 2 * way > 350)
		way *= -1;
	
	if (iter < 2 * way)
		way *= 1;
		

	draw(display, display->w/2, display->h/2, l, iter);

	return TIMER_TICK;
}

int main(void)
{
	SDL_Event ev;

	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0)
		return -1;

	display = SDL_SetVideoMode(DISPLAY_W, DISPLAY_H, 0, SDL_SWSURFACE | SDL_RESIZABLE | SDL_DOUBLEBUF);

	if (display == NULL) {
		SDL_Quit();
		return -1;
	}

	GP_SDL_ContextFromSurface(&context, display);

	iter = 0;
	draw(display, display->w/2, display->h/2, l, iter);

	timer = SDL_AddTimer(0, timer_callback, NULL);

	if (timer == 0) {
		SDL_Quit();
		return -1;
	}

	while (SDL_WaitEvent(&ev) > 0) {
		switch (ev.type) {
			case SDL_KEYDOWN:
			case SDL_QUIT:
				SDL_Quit();
				return 0;
			break;
			default:
			break;
		}
	}

	SDL_Quit();

	return 0;
}

