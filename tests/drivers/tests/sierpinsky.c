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

#include "GP.h"
#include "GP_Framebuffer.h"

#include <math.h>

#define TIMER_TICK 50
#define DISPLAY_W 640
#define DISPLAY_H 480
#define sqr(x) ((x)*(x))
#define sgn(x) ((x)>0 ? 1 : -1)

int iter, l, way = 1;

GP_Pixel black, blue, gray;

static void sierpinsky(GP_Context *context, GP_Pixel pixel, float x1, float y1, float x4, float y4, int iter)
{
	float x2, y2, x3, y3, x5, y5;
	
	if (iter <= 0) {
		GP_Line(context, x1, y1, x4, y4, black);
		return;
	}

	x2 = floor((2*x1 + x4)/3);
	y2 = floor((2*y1 + y4)/3);

	x3 = floor((2*x4 + x1)/3);
	y3 = floor((2*y4 + y1)/3);

	x5 = (x1+x4)/2 + (y2 - y3)*sqrt(3.00/4);
	y5 = (y1+y4)/2 + (x3 - x2)*sqrt(3.00/4);

	GP_FillTriangle(context, x2, y2, x3, y3, x5, y5, pixel);

	sierpinsky(context, pixel, x1, y1, x2, y2, iter - 1);
	sierpinsky(context, pixel, x2, y2, x5, y5, iter - 1);
	sierpinsky(context, pixel, x5, y5, x3, y3, iter - 1);
	sierpinsky(context, pixel, x3, y3, x4, y4, iter - 1);
}

static void draw(GP_Context *context, int x, int y, int l, int iter)
{
	float x1, y1, x2, y2, x3, y3;
	int w = context->w;
	int h = context->h;

	l = ((w < h ? w : h) - 20)/(5 - 1.00*iter/120);
	
	x1 = sin(1.00 * iter/57) * l + x;
	y1 = cos(1.00 * iter/57) * l + y;
	
	x2 = sin(1.00 * (iter+120)/57) * l + x;
	y2 = cos(1.00 * (iter+120)/57) * l + y;
	
	x3 = sin(1.00 * (iter+240)/57) * l + x;
	y3 = cos(1.00 * (iter+240)/57) * l + y;

	GP_Fill(context, gray);

	GP_FillTriangle(context, x1, y1, x2, y2, x3, y3, blue);

	sierpinsky(context, blue, x1, y1, x2, y2, iter/60%6);
	sierpinsky(context, blue, x2, y2, x3, y3, iter/60%6);
	sierpinsky(context, blue, x3, y3, x1, y1, iter/60%6);
}

static void redraw(GP_Context *context)
{
	iter += 2 * way;
	
	if (iter + 2 * way > 350)
		way *= -1;
	
	if (iter < 2 * way)
		way *= 1;
		

	draw(context, context->w/2, context->h/2, l, iter);
}

int main(void)
{
	GP_Framebuffer *fb = GP_FramebufferInit("/dev/fb0");
	GP_Context *context;
	int i;

	if (fb == NULL)
		return 1;

	context = &fb->context;

	GP_ColorNameToPixel(context, GP_COL_GRAY_LIGHT, &gray);
	GP_ColorNameToPixel(context, GP_COL_BLUE, &blue);
	GP_ColorNameToPixel(context, GP_COL_BLACK, &black);

	iter = 0;
	draw(context, context->w/2, context->h/2, l, iter);

	for (i = 0; i < 300; i++) {
		redraw(context);
		usleep(10000);
	}

	GP_FramebufferExit(fb);

	return 0;
}

