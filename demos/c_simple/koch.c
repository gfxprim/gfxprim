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
 * Copyright (C) 2009-2013 Cyril Hrubis <metan@ucw.cz>                       *
 *                                                                           *
 *****************************************************************************/

/*

  Simple test for triangle drawing runtime.

 */

#include <math.h>

#include <GP.h>

#define TIMER_TICK 20000
#define DISPLAY_W 640
#define DISPLAY_H 480
#define sqr(x) ((x)*(x))
#define sgn(x) ((x)>0 ? 1 : -1)

static GP_Backend *backend;
static GP_Context *context;

static int iter, l, way = 1, draw_edge = 1;
static GP_Pixel black, blue, gray, red;

static void sierpinsky(double x1, double y1, double x4, double y4, int iter)
{
	double x2, y2, x3, y3, x5, y5;
	GP_Pixel pixel;
	pixel = GP_RGBToPixel(0, 0, 255-16*iter, context->pixel_type);

	if (iter <= 0) {
		if (draw_edge)
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

	GP_PutPixel(context, x2, y2, red);
	GP_PutPixel(context, x3, y3, red);
	GP_PutPixel(context, x5, y5, red);

	sierpinsky(x1, y1, x2, y2, iter - 1);
	sierpinsky(x2, y2, x5, y5, iter - 1);
	sierpinsky(x5, y5, x3, y3, iter - 1);
	sierpinsky(x3, y3, x4, y4, iter - 1);
}

static void draw(int x, int y, int l, int iter)
{
	double x1, y1, x2, y2, x3, y3;
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

	sierpinsky(x1, y1, x2, y2, iter/60%6);
	sierpinsky(x2, y2, x3, y3, iter/60%6);
	sierpinsky(x3, y3, x1, y1, iter/60%6);

	GP_BackendFlip(backend);
}

static int paused = 0;

void redraw(void)
{
	if (paused)
		return;

	iter += 2 * way;

	if (iter > 350)
		way *= -1;

	if (iter < 0)
		way *= -1;

	draw(context->w/2, context->h/2, l, iter);
}

int main(void)
{
	const char *backend_opts = "X11";

	backend = GP_BackendInit(backend_opts, "Koch");

	if (backend == NULL) {
		fprintf(stderr, "Failed to initalize backend '%s'\n",
		        backend_opts);
		return 1;
	}

	context = backend->context;

	black = GP_RGBToContextPixel(0x00, 0x00, 0x00, context);
	blue  = GP_RGBToContextPixel(0x00, 0x00, 0xff, context);
	gray  = GP_RGBToContextPixel(0xbe, 0xbe, 0xbe, context);
	red   = GP_RGBToContextPixel(0xff, 0x00, 0x00, context);

	iter = 0;
	draw(context->w/2, context->h/2, l, iter);

	for (;;) {
		GP_Event ev;

		redraw();

		GP_BackendPoll(backend);

		while (GP_BackendGetEvent(backend, &ev)) {
			GP_EventDump(&ev);

			switch (ev.type) {
			case GP_EV_KEY:
				if (ev.code != GP_EV_KEY_DOWN)
					continue;

				switch (ev.val.key.key) {
				case GP_KEY_P:
					paused = !paused;
				break;
				case GP_KEY_E:
					draw_edge = !draw_edge;
				break;
				case GP_KEY_ESC:
					GP_BackendExit(backend);
					return 0;
				break;
				}
			break;
			case GP_EV_SYS:
				if (ev.code == GP_EV_SYS_RESIZE)
					GP_BackendResizeAck(backend);
			}
		}
		usleep(TIMER_TICK);
	}

	return 0;
}
