// SPDX-License-Identifier: GPL-2.1-or-later
/*
 * Copyright (C) 2009-2010 Jiri "BlueBear" Dluhos
 *                         <jiri.bluebear.dluhos@gmail.com>
 *
 * Copyright (C) 2009-2026 Cyril Hrubis <metan@ucw.cz>
 */

/*

  Simple test for triangle drawing runtime.

 */

#include <math.h>

#include <gfxprim.h>

#define TIMER_TICK 20000
#define sqr(x) ((x)*(x))
#define sgn(x) ((x)>0 ? 1 : -1)

static gp_backend *backend;

static int iter, l, way = 1, draw_edge = 1;
static gp_pixel black, blue, gray, red;

static void sierpinsky(double x1, double y1, double x4, double y4, int iter)
{
	gp_pixmap *pixmap = backend->pixmap;
	double x2, y2, x3, y3, x5, y5;
	gp_pixel pixel;
	pixel = gp_rgb_to_pixel(0, 0, 255-16*iter, pixmap->pixel_type);

	if (iter <= 0) {
		if (draw_edge)
			gp_line(pixmap, x1, y1, x4, y4, black);
		return;
	}

	x2 = floor((2*x1 + x4)/3);
	y2 = floor((2*y1 + y4)/3);

	x3 = floor((2*x4 + x1)/3);
	y3 = floor((2*y4 + y1)/3);

	x5 = (x1+x4)/2 + (y2 - y3)*sqrt(3.00/4);
	y5 = (y1+y4)/2 + (x3 - x2)*sqrt(3.00/4);

	gp_fill_triangle(pixmap, x2, y2, x3, y3, x5, y5, pixel);

	gp_putpixel(pixmap, x2, y2, red);
	gp_putpixel(pixmap, x3, y3, red);
	gp_putpixel(pixmap, x5, y5, red);

	sierpinsky(x1, y1, x2, y2, iter - 1);
	sierpinsky(x2, y2, x5, y5, iter - 1);
	sierpinsky(x5, y5, x3, y3, iter - 1);
	sierpinsky(x3, y3, x4, y4, iter - 1);
}

static void draw(int x, int y, int l, int iter)
{
	gp_pixmap *pixmap = backend->pixmap;
	double x1, y1, x2, y2, x3, y3;
	int w = gp_backend_w(backend);
	int h = gp_backend_h(backend);

	l = ((w < h ? w : h) - 20)/(5 - 1.00*iter/120);

	x1 = sin(1.00 * iter/57) * l + x;
	y1 = cos(1.00 * iter/57) * l + y;

	x2 = sin(1.00 * (iter+120)/57) * l + x;
	y2 = cos(1.00 * (iter+120)/57) * l + y;

	x3 = sin(1.00 * (iter+240)/57) * l + x;
	y3 = cos(1.00 * (iter+240)/57) * l + y;

	gp_fill(pixmap, gray);

	gp_fill_triangle(pixmap, x1, y1, x2, y2, x3, y3, blue);

	sierpinsky(x1, y1, x2, y2, iter/60%6);
	sierpinsky(x2, y2, x3, y3, iter/60%6);
	sierpinsky(x3, y3, x1, y1, iter/60%6);

	gp_backend_flip(backend);
}

static int paused = 0;

void redraw(void)
{
	if (paused || !backend->pixmap)
		return;

	iter += 2 * way;

	if (iter > 350)
		way *= -1;

	if (iter < 0)
		way *= -1;

	draw(gp_backend_w(backend)/2, gp_backend_h(backend)/2, l, iter);
}

static void init_colors(gp_pixel_type pixel_type)
{
	black = gp_rgb_to_pixel(0x00, 0x00, 0x00, pixel_type);
	blue  = gp_rgb_to_pixel(0x00, 0x00, 0xff, pixel_type);
	gray  = gp_rgb_to_pixel(0xbe, 0xbe, 0xbe, pixel_type);
	red   = gp_rgb_to_pixel(0xff, 0x00, 0x00, pixel_type);
}

int main(int argc, char *argv[])
{
	const char *backend_opts = NULL;
	int opt;

	while ((opt = getopt(argc, argv, "b:h")) != -1) {
		switch (opt) {
		case 'b':
			backend_opts = optarg;
		break;
		case 'h':
			printf("Usage: %s [-b backend]\n\n", argv[0]);
			gp_backend_init_help();
			return 0;
		default:
			fprintf(stderr, "Invalid paramter '%c'\n", opt);
			return 1;
		}
	}

	backend = gp_backend_init(backend_opts, 0, 0, "Koch");

	if (!backend) {
		fprintf(stderr, "Failed to initalize backend '%s'\n",
		        backend_opts);
		return 1;
	}

	iter = 0;

	for (;;) {
		gp_event *ev;

		redraw();

		gp_backend_poll(backend);

		while ((ev = gp_backend_ev_get(backend))) {
			gp_ev_dump(ev);

			switch (ev->type) {
			case GP_EV_KEY:
				if (ev->code != GP_EV_KEY_DOWN)
					continue;

				switch (ev->key.key) {
				case GP_KEY_P:
					paused = !paused;
				break;
				case GP_KEY_E:
					draw_edge = !draw_edge;
				break;
				case GP_KEY_ESC:
					gp_backend_exit(backend);
					return 0;
				break;
				}
			break;
			case GP_EV_SYS:
				switch (ev->code) {
				case GP_EV_SYS_RENDER_STOP:
					gp_backend_render_stopped(backend);
				break;
				case GP_EV_SYS_RENDER_PIXEL_TYPE:
					init_colors(ev->pixel_type);
				break;
				case GP_EV_SYS_QUIT:
					gp_backend_exit(backend);
					return 0;
				}
			}
		}
		usleep(TIMER_TICK);
	}

	return 0;
}
