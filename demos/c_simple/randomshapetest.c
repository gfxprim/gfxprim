// SPDX-License-Identifier: GPL-2.1-or-later
/*
 * Copyright (C) 2009-2010 Jiri "BlueBear" Dluhos
 *                         <jiri.bluebear.dluhos@gmail.com>
 *
 * Copyright (C) 2009-2013 Cyril Hrubis <metan@ucw.cz>
 */

#include <stdio.h>
#include <stdlib.h>

#include <gfxprim.h>

static gp_backend *win;

/* Globally used colors. */
static gp_pixel white, black;

/* Holding flag (pauses drawing). */
static int pause_flag = 0;

/* Shape to draw */
#define SHAPE_FIRST	1
#define SHAPE_CIRCLE	1
#define SHAPE_ELLIPSE	2
#define SHAPE_TRIANGLE	3
#define SHAPE_RECTANGLE	4
#define SHAPE_TETRAGON   5
#define SHAPE_POLYGON	6
#define SHAPE_LAST	6
static int shape = SHAPE_FIRST;

/* Draw outlines? */
static int outline_flag = 0;

/* Draw filled shapes? */
static int fill_flag = 1;

/* Do a clipping test? */
static int cliptest_flag = 0;

void random_point(const gp_pixmap *c, int *x, int *y)
{
	if (cliptest_flag) {
		*x = random() % (3*c->w) - c->w;
		*y = random() % (3*c->h) - c->h;
	} else {
		*x = random() % c->w;
		*y = random() % c->h;
	}
}

void random_point_AA(const gp_pixmap *c, int *x, int *y)
{
	*x = random() % (c->w<<8);
	*y = random() % (c->h<<8);
}

void draw_random_circle(gp_pixel pixel)
{
	int x, y;
	random_point(win->pixmap, &x, &y);
	int r = random() % 50;

	if (fill_flag)
		gp_fill_circle(win->pixmap, x, y, r, pixel);

	if (outline_flag)
		gp_circle(win->pixmap, x, y, r, white);
}

void draw_random_ellipse(gp_pixel pixel)
{
	int x, y;
	random_point(win->pixmap, &x, &y);
	int rx = random() % 50;
	int ry = random() % 50;

	if (fill_flag)
		gp_fill_ellipse(win->pixmap, x, y, rx, ry, pixel);

	if (outline_flag)
		gp_ellipse(win->pixmap, x, y, rx, ry, white);
}

void draw_random_triangle(gp_pixel pixel)
{
	int x0, y0, x1, y1, x2, y2;
	random_point(win->pixmap, &x0, &y0);
	random_point(win->pixmap, &x1, &y1);
	random_point(win->pixmap, &x2, &y2);

	if (fill_flag)
		gp_fill_triangle(win->pixmap, x0, y0, x1, y1, x2, y2, pixel);

	if (outline_flag)
		gp_triangle(win->pixmap, x0, y0, x1, y1, x2, y2, white);
}

void draw_random_rectangle(gp_pixel pixel)
{
	int x0, y0, x1, y1;
	random_point(win->pixmap, &x0, &y0);
	random_point(win->pixmap, &x1, &y1);

	if (fill_flag)
		gp_fill_rect(win->pixmap, x0, y0, x1, y1, pixel);

	if (outline_flag)
		gp_rect(win->pixmap, x0, y0, x1, y1, white);
}

void draw_random_tetragon(gp_pixel pixel)
{
	int x0, y0, x1, y1, x2, y2, x3, y3;
	random_point(win->pixmap, &x0, &y0);
	random_point(win->pixmap, &x1, &y1);
	random_point(win->pixmap, &x2, &y2);
	random_point(win->pixmap, &x3, &y3);

	if (fill_flag)
		gp_fill_tetragon(win->pixmap, x0, y0, x1, y1, x2, y2, x3, y3, pixel);

	if (outline_flag)
		gp_tetragon(win->pixmap, x0, y0, x1, y1, x2, y2, x3, y3, pixel);
}

void draw_random_polygon(gp_pixel pixel)
{
	gp_coord xy[10];
	int i;

	for (i = 0; i < 5; i++) {
		random_point(win->pixmap, xy + 2*i, xy + 2*i + 1);
	}

	gp_fill_polygon_raw(win->pixmap, 5, xy, pixel);
}

void clear_screen(void)
{
	gp_fill(win->pixmap, black);
	gp_backend_flip(win);
}

void redraw_screen(void)
{
	if (pause_flag)
		return;

	/* Pick a random color for drawing. */
	gp_pixel pixel;
	pixel = gp_rgb_to_pixel(random() % 256, random() % 256,
	                      random() % 256, win->pixmap->pixel_type);

	switch (shape) {
	case SHAPE_CIRCLE:
		draw_random_circle(pixel);
	break;
	case SHAPE_ELLIPSE:
		draw_random_ellipse(pixel);
	break;
	case SHAPE_TRIANGLE:
		draw_random_triangle(pixel);
	break;
	case SHAPE_RECTANGLE:
		draw_random_rectangle(pixel);
	break;
	case SHAPE_TETRAGON:
		draw_random_tetragon(pixel);
	break;
	case SHAPE_POLYGON:
		draw_random_polygon(pixel);
	break;
	}
}

void event_loop(void)
{
	gp_event ev;

	while (gp_backend_get_event(win, &ev)) {
		switch (ev.type) {
		case GP_EV_KEY:
			if (ev.code != GP_EV_KEY_DOWN)
				continue;

			switch (ev.val.key.key) {
			case GP_KEY_SPACE:
				shape++;
				if (shape > SHAPE_LAST)
					shape = SHAPE_FIRST;
				clear_screen();
				pause_flag = 0;
			break;
			case GP_KEY_P:
				pause_flag = !pause_flag;
			break;
			case GP_KEY_F:
				fill_flag = !fill_flag;
				if (!fill_flag && !outline_flag)
					outline_flag = 1;
			break;
			case GP_KEY_O:
				outline_flag = !outline_flag;
				if (!fill_flag && !outline_flag)
					fill_flag = 1;
			break;
			case GP_KEY_C:
				cliptest_flag = !cliptest_flag;
			break;
			case GP_KEY_X:
				clear_screen();
			break;
			case GP_KEY_ESC:
				gp_backend_exit(win);
				exit(0);
			break;
			}
		break;
		case GP_EV_SYS:
			if (ev.code == GP_EV_SYS_RESIZE) {
				gp_backend_resize_ack(win);
				clear_screen();
				gp_backend_flip(win);
			}
		break;
		}
	}
}

int main(int argc, char *argv[])
{
	const char *backend_opts = "X11";
	int opt;

	while ((opt = getopt(argc, argv, "b:h")) != -1) {
		switch (opt) {
		case 'b':
			backend_opts = optarg;
		break;
		case 'h':
			printf("Usage: %s [-b backend]\n\n", argv[0]);
			gp_backend_init(NULL, NULL);
			return 0;
		default:
			fprintf(stderr, "Invalid paramter '%c'\n", opt);
			return 1;
		}
	}

	win = gp_backend_init(backend_opts, "Random Shape Test");

	if (win == NULL) {
		fprintf(stderr, "Failed to initalize backend '%s'\n",
		        backend_opts);
		return 1;
	}

	white = gp_rgb_to_pixmap_pixel(0xff, 0xff, 0xff, win->pixmap);
	black = gp_rgb_to_pixmap_pixel(0x00, 0x00, 0x00, win->pixmap);

	for (;;) {
		gp_backend_poll(win);
		event_loop();

		usleep(20000);

		if (pause_flag)
			continue;

		redraw_screen();
		gp_backend_flip(win);
	}
}

