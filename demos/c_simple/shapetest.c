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

static gp_pixmap *win;
static gp_backend *backend;

/* Basic colors in display-specific format. */
static gp_pixel black, white, yellow, green, red, gray, darkgray;

/* Radius of the shape being drawn */
static int xradius = 5;
static int yradius = 5;

/* Draw outline? 0=none, 1=before filling, 2=after filling */
static int outline = 0;

/* Fill the shape? */
static int fill = 1;

/* Show axes? */
static int show_axes = 1;

/* Shape to be drawn */
#define SHAPE_FIRST	1
#define SHAPE_TRIANGLE	1
#define SHAPE_CIRCLE    2
#define SHAPE_RING	3
#define SHAPE_ELLIPSE	4
#define SHAPE_RECTANGLE	5
#define SHAPE_TETRAGON  6
#define SHAPE_POLYGON   7
#define SHAPE_ARC	8
#define SHAPE_LAST	8
static int shape = SHAPE_FIRST;

/* Variants in coordinates, if applicable */
static int variant = 1;

/* center of drawing */
static int center_x;
static int center_y;

void draw_testing_triangle(int x, int y, int xradius, int yradius)
{
	int x0, y0, x1, y1, x2, y2;
	switch (variant) {
	case 1:
		x0 = x;
		y0 = y - yradius;
		x1 = x - xradius;
		y1 = y;
		x2 = x + xradius;
		y2 = y + yradius;
		break;
	case 2:
		x0 = x - xradius;
		y0 = y - yradius;
		x1 = x + xradius;
		y1 = y;
		x2 = x + xradius;
		y2 = y + yradius;
		break;
	case 3:
		x0 = x - xradius;
		y0 = y - yradius;
		x1 = x + xradius;
		y1 = y + yradius;
		x2 = x - xradius + xradius/8;
		y2 = y;
		break;
	case 4:
	default:
		x0 = x;
		y0 = y - yradius;
		x1 = x + xradius;
		y1 = y + yradius;
		x2 = x - xradius;
		y2 = y + yradius;
		break;
	}

	/* draw the three vertices green; they should never be visible
	 * because the red triangle should cover them; if they are visible,
	 * it means we don't draw to the end */
	gp_putpixel(win, x0, y0, green);
	gp_putpixel(win, x1, y1, green);
	gp_putpixel(win, x2, y2, green);

	if (outline == 1)
		gp_triangle(win, x0, y0, x1, y1, x2, y2, yellow);

	if (fill)
		gp_fill_triangle(win, x0, y0, x1, y1, x2, y2, red);

	if (outline == 2)
		gp_triangle(win, x0, y0, x1, y1, x2, y2, white);
}

void draw_testing_circle(int x, int y, int xradius,
			__attribute__((unused)) int yradius)
{
	if (outline == 1)
		gp_circle(win, x, y, xradius, yellow);

	if (fill)
		gp_fill_circle(win, x, y, xradius, red);

	if (outline == 2)
		gp_circle(win, x, y, xradius, white);
}

void draw_testing_ring(int x, int y, int xradius,
			__attribute__((unused)) int yradius)
{
	if (outline == 1)
		gp_ring(win, x, y, xradius, yradius, yellow);

	if (fill)
		gp_fill_ring(win, x, y, xradius, yradius, red);

	if (outline == 2)
		gp_ring(win, x, y, xradius, yradius, white);
}

void draw_testing_ellipse(int x, int y, int xradius, int yradius)
{
	if (outline == 1)
		gp_ellipse(win, x, y, xradius, yradius, yellow);

	if (fill)
		gp_fill_ellipse(win, x, y, xradius, yradius, red);

	if (outline == 2)
		gp_ellipse(win, x, y, xradius, yradius, white);
}

void draw_testing_arc(int x, int y, int xradius, int yradius)
{
	gp_arc_segment(win, x, y, xradius, yradius, -1,
			M_PI - M_PI/8.0, M_PI/4.0, red);
}

void draw_testing_rectangle(int x, int y, int xradius, int yradius)
{
	int x0 = x - xradius, y0 = y - yradius;
	int x1 = x + xradius, y1 = y + yradius;

	if (outline == 1)
		gp_rect(win, x0, y0, x1, y1, yellow);

	if (fill)
		gp_fill_rect(win, x0, y0, x1, y1, red);

	if (outline == 2)
		gp_rect(win, x0, y0, x1, y1, white);
}

void draw_testing_tetragon(int x, int y, int xradius, int yradius)
{
	int x0 = x - xradius, y0 = y - yradius;
	int x1 = x + xradius, y1 = y;
	int x2 = x + xradius, y2 = y + yradius/2;
	int x3 = x,           y3 = y + yradius;

	if (outline == 1)
		gp_tetragon(win, x0, y0, x1, y1, x2, y2, x3, y3, yellow);

	if (fill)
		gp_fill_tetragon(win, x0, y0, x1, y1, x2, y2, x3, y3, red);

	if (outline == 2)
		gp_tetragon(win, x0, y0, x1, y1, x2, y2, x3, y3, white);
}

void draw_testing_polygon(int x, int y, int xradius, int yradius)
{
	gp_coord xy[14];
	unsigned int edges = 7;

	xy[0] = x + xradius;
	xy[1] = y;

	xy[2] = x + 3 * xradius / 4;
	xy[3] = y + yradius / 4;

	xy[4] = x + 3 * xradius / 4;
	xy[5] = y + 3 * yradius / 4;

	xy[6] = x + xradius / 4;
	xy[7] = y + 3 * yradius / 4;

	xy[8] = x;
	xy[9] = y;

	xy[10] = x - xradius;
	xy[11] = y;

	xy[12] = x - 3 * xradius / 4;
	xy[13] = y - yradius / 4;

	if (outline == 1)
		gp_polygon(win, edges, xy, yellow);

	if (fill)
		gp_fill_polygon(win, edges, xy, red);

	if (outline == 2)
		gp_polygon(win, edges, xy, white);
}

void redraw_screen(void)
{

	/* text style for the label */
	gp_text_style style = {
		.font = gp_font_tiny,
		.pixel_xmul = 6,
		.pixel_ymul = 6,
		.pixel_xspace = 2,
		.pixel_yspace = 2,
	};

	gp_fill(win, black);

	/* axes */
	if (show_axes) {
		int w, h;

		w = gp_pixmap_w(win);
		h = gp_pixmap_h(win);

		gp_hline(win, 0, w, center_y, gray);
		gp_hline(win, 0, w, center_y-yradius, darkgray);
		gp_hline(win, 0, w, center_y+yradius, darkgray);
		gp_vline(win, center_x, 0, h, gray);
		gp_vline(win, center_x-xradius, 0, h, darkgray);
		gp_vline(win, center_x+xradius, 0, h, darkgray);
	}

	/* the shape */
	const char *title = NULL;
	switch (shape) {
	case SHAPE_TRIANGLE:
		draw_testing_triangle(center_x, center_y, xradius, yradius);
		title = "TRIANGLE";
	break;
	case SHAPE_CIRCLE:
		draw_testing_circle(center_x, center_y, xradius, yradius);
		title = "CIRCLE";
	break;
	case SHAPE_RING:
		draw_testing_ring(center_x, center_y, xradius, yradius);
		title = "RING";
	break;
	case SHAPE_ELLIPSE:
		draw_testing_ellipse(center_x, center_y, xradius, yradius);
		title = "ELLIPSE";
	break;
	case SHAPE_RECTANGLE:
		draw_testing_rectangle(center_x, center_y, xradius, yradius);
		title = "RECTANGLE";
	break;
	case SHAPE_TETRAGON:
		draw_testing_tetragon(center_x, center_y, xradius, yradius);
		title = "TETRAGON";
	break;
	case SHAPE_POLYGON:
		draw_testing_polygon(center_x, center_y, xradius, yradius);
		title = "POLYGON";
	break;
	case SHAPE_ARC:
		draw_testing_arc(center_x, center_y, xradius, yradius);
		title = "ARC";
	break;
	}

	gp_text(win, &style, 16, 16, GP_ALIGN_RIGHT|GP_VALIGN_BELOW,
	        yellow, black, title);

	gp_backend_flip(backend);
}

static void xradius_add(int xradius_add)
{
	if (xradius + xradius_add > 1 &&
	    xradius + xradius_add < (int) gp_pixmap_w(win))
		xradius += xradius_add;
}


static void yradius_add(int yradius_add)
{
	if (yradius + yradius_add > 1 &&
	    yradius + yradius_add < (int) gp_pixmap_h(win))
		yradius += yradius_add;
}

static void xcenter_add(int xcenter_add)
{
	if (center_x + xcenter_add > 1 &&
	    center_x + xcenter_add < (int) gp_pixmap_w(win)/2)
		center_x += xcenter_add;
}

static void ycenter_add(int ycenter_add)
{
	if (center_y + ycenter_add > 1 &&
	    center_y + ycenter_add < (int) gp_pixmap_h(win)/2)
		center_y += ycenter_add;
}

void event_loop(void)
{
	int shift_pressed;

	gp_event ev;

	for (;;) {
		gp_backend_wait_event(backend, &ev);

		//gp_eventDump(&ev);

		shift_pressed = gp_event_get_key(&ev, GP_KEY_LEFT_SHIFT) ||
		                gp_event_get_key(&ev, GP_KEY_RIGHT_SHIFT);

		switch (ev.type) {
		case GP_EV_KEY:
			if (ev.code != GP_EV_KEY_DOWN)
				continue;

			switch (ev.val.key.key) {
			case GP_KEY_X:
				win->x_swap = !win->x_swap;
			break;
			case GP_KEY_Y:
				win->y_swap = !win->y_swap;
			break;
			case GP_KEY_R:
				win->axes_swap = !win->axes_swap;
				center_x = gp_pixmap_w(win) / 2;
				center_y = gp_pixmap_h(win) / 2;
			break;
			case GP_KEY_F:
				fill = !fill;
				if (!fill && !outline)
					outline = 1;
			break;
			case GP_KEY_O:
				outline++;
				if (outline == 3)
					outline = 0;
				if (!fill && outline == 0)
					fill = 1;
			break;
			case GP_KEY_A:
				show_axes = !show_axes;
			break;
			case GP_KEY_LEFT:
				if (shift_pressed)
					xcenter_add(-1);
				else
					xradius_add(-1);
			break;
			case GP_KEY_RIGHT:
				if (shift_pressed)
					xcenter_add(1);
				else
					xradius_add(1);
			break;
			case GP_KEY_UP:
				if (shift_pressed)
					ycenter_add(-1);
				else
					yradius_add(1);
			break;
			case GP_KEY_DOWN:
				if (shift_pressed)
					ycenter_add(1);
				else
					yradius_add(-1);
			break;
			case GP_KEY_SPACE:
				shape++;
				if (shape > SHAPE_LAST)
					shape = SHAPE_FIRST;
			break;
			case GP_KEY_EQUAL:
				if (xradius > yradius)
					yradius = xradius;
				else
					xradius = yradius;
			break;
			case GP_KEY_1:
				variant = 1;
			break;
			case GP_KEY_2:
				variant = 2;
			break;
			case GP_KEY_3:
				variant = 3;
			break;
			case GP_KEY_4:
				variant = 4;
			break;
			case GP_KEY_PAGE_UP:
				xradius_add(1);
				yradius_add(1);
			break;
			case GP_KEY_PAGE_DOWN:
				xradius_add(-1);
				yradius_add(-1);
			break;
			case GP_KEY_ESC:
				gp_backend_exit(backend);
				exit(0);
			break;
			}
		break;
		case GP_EV_SYS:
			switch(ev.code) {
			case GP_EV_SYS_QUIT:
				gp_backend_exit(backend);
				exit(0);
			break;
			case GP_EV_SYS_RESIZE:
				gp_backend_resize_ack(backend);
				win = backend->pixmap;
				center_x = gp_pixmap_w(win) / 2;
				center_y = gp_pixmap_h(win) / 2;
			break;
			}
		break;
		}

		redraw_screen();
	}
}

void print_instructions(void)
{
	printf("Use the following keys to control the test:\n");
	printf("    Esc ................. exit\n");
	printf("    Space ............... change shapes\n");
	printf("    O ................... draw outlines (none/before/after fill)\n");
	printf("    F ................... toggle filling\n");
	printf("    A ................... show/hide axes\n");
	printf("    X ................... mirror X\n");
	printf("    Y ................... mirror Y\n");
	printf("    R ................... reverse X and Y\n");
	printf("    left/right .......... increase/decrease horizontal radius\n");
	printf("    up/down ............. increase/decrease vertical radius\n");
	printf("    shift + left/right .. increase/decrease horizontal center\n");
	printf("    shift + up/down ..... increase/decrease vertical center\n");
	printf("    PgUp/PgDn ........... increase/decrease both radii\n");
	printf("    = ................... reset radii to the same value\n");
	printf("    1/2/3/4 ............. choose shape variant (if applicable)\n");
}

int main(int argc, char *argv[])
{
	const char *backend_opts = "X11";
	int opt;

	while ((opt = getopt(argc, argv, "b:")) != -1) {
		switch (opt) {
		case 'b':
			backend_opts = optarg;
		break;
		default:
			fprintf(stderr, "Invalid paramter '%c'\n", opt);
		}
	}

	backend = gp_backend_init(backend_opts, "Shapetest");

	if (backend == NULL) {
		fprintf(stderr, "Failed to initalize backend '%s'\n",
		        backend_opts);
		return 1;
	}

	win = backend->pixmap;

	center_x = win->w / 2;
	center_y = win->h / 2;

	/* Load colors compatible with the display */
	black    = gp_rgb_to_pixmap_pixel(0x00, 0x00, 0x00, win);
	white    = gp_rgb_to_pixmap_pixel(0xff, 0xff, 0xff, win);
	yellow   = gp_rgb_to_pixmap_pixel(0xff, 0xff, 0x00, win);
	green    = gp_rgb_to_pixmap_pixel(0x00, 0xff, 0x00, win);
	red      = gp_rgb_to_pixmap_pixel(0xff, 0x00, 0x00, win);
	gray     = gp_rgb_to_pixmap_pixel(0xbe, 0xbe, 0xbe, win);
	darkgray = gp_rgb_to_pixmap_pixel(0x7f, 0x7f, 0x7f, win);

	print_instructions();
	redraw_screen();
	event_loop();

	return 0;
}
