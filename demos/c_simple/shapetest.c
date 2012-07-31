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

#include <GP.h>

static GP_Context *win;
static GP_Backend *backend;

/* Basic colors in display-specific format. */
static GP_Pixel black, white, yellow, green, red, gray, darkgray;

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
#define SHAPE_ARC	7
#define SHAPE_LAST	7
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
	GP_PutPixel(win, x0, y0, green);
	GP_PutPixel(win, x1, y1, green);
	GP_PutPixel(win, x2, y2, green);

	if (outline == 1)
		GP_Triangle(win, x0, y0, x1, y1, x2, y2, yellow);

	if (fill)
		GP_FillTriangle(win, x0, y0, x1, y1, x2, y2, red);

	if (outline == 2)
		GP_Triangle(win, x0, y0, x1, y1, x2, y2, white);
}

void draw_testing_circle(int x, int y, int xradius,
			__attribute__((unused)) int yradius)
{
	if (outline == 1)
		GP_Circle(win, x, y, xradius, yellow);

	if (fill)
		GP_FillCircle(win, x, y, xradius, red);

	if (outline == 2)
		GP_Circle(win, x, y, xradius, white);
}

void draw_testing_ring(int x, int y, int xradius,
			__attribute__((unused)) int yradius)
{
	if (outline == 1)
		GP_Ring(win, x, y, xradius, yradius, yellow);

	if (fill)
		GP_FillRing(win, x, y, xradius, yradius, red);

	if (outline == 2)
		GP_Ring(win, x, y, xradius, yradius, white);
}

void draw_testing_ellipse(int x, int y, int xradius, int yradius)
{
	if (outline == 1)
		GP_Ellipse(win, x, y, xradius, yradius, yellow);

	if (fill)
		GP_FillEllipse(win, x, y, xradius, yradius, red);

	if (outline == 2)
		GP_Ellipse(win, x, y, xradius, yradius, white);
}

void draw_testing_arc(int x, int y, int xradius, int yradius)
{
	GP_ArcSegment(win, x, y, xradius, yradius, -1,
			M_PI - M_PI/8.0, M_PI/4.0, red);
}

void draw_testing_rectangle(int x, int y, int xradius, int yradius)
{
	int x0 = x - xradius, y0 = y - yradius;
	int x1 = x + xradius, y1 = y + yradius;
	
	if (outline == 1)
		GP_Rect(win, x0, y0, x1, y1, yellow);

	if (fill)
		GP_FillRect(win, x0, y0, x1, y1, red);

	if (outline == 2)
		GP_Rect(win, x0, y0, x1, y1, white);
}

void draw_testing_tetragon(int x, int y, int xradius, int yradius)
{
	int x0 = x - xradius, y0 = y - yradius;
	int x1 = x + xradius, y1 = y;
	int x2 = x + xradius, y2 = y + yradius/2;
	int x3 = x,           y3 = y + yradius;

	if (outline == 1)
		GP_Tetragon(win, x0, y0, x1, y1, x2, y2, x3, y3, yellow);
	
	if (fill)
		GP_FillTetragon(win, x0, y0, x1, y1, x2, y2, x3, y3, red);

	if (outline == 2)
		GP_Tetragon(win, x0, y0, x1, y1, x2, y2, x3, y3, white);
}

void redraw_screen(void)
{

	/* text style for the label */
	GP_TextStyle style = {
		.font = &GP_DefaultConsoleFont,
		.pixel_xmul = 2,
		.pixel_ymul = 1,
		.pixel_xspace = 0,
		.pixel_yspace = 1,
	};

	GP_Fill(win, black);

	/* axes */
	if (show_axes) {
		GP_HLine(win, 0, win->w, center_y, gray);
		GP_HLine(win, 0, win->w, center_y-yradius, darkgray);
		GP_HLine(win, 0, win->w, center_y+yradius, darkgray);
		GP_VLine(win, center_x, 0, win->h, gray);
		GP_VLine(win, center_x-xradius, 0, win->h, darkgray);
		GP_VLine(win, center_x+xradius, 0, win->h, darkgray);
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
	case SHAPE_ARC:
		draw_testing_arc(center_x, center_y, xradius, yradius);
		title = "ARC";
		break;
	}

	GP_Text(win, &style, 16, 16, GP_ALIGN_RIGHT|GP_VALIGN_BELOW,
	        white, black, title);

	GP_BackendFlip(backend);
}

static void xradius_add(int xradius_add)
{
	if (xradius + xradius_add > 1 &&
	    xradius + xradius_add < (int)win->w)
		xradius += xradius_add;
}
	

static void yradius_add(int yradius_add)
{
	if (yradius + yradius_add > 1 &&
	    yradius + yradius_add < (int)win->h)
		yradius += yradius_add;
}
			
static void xcenter_add(int xcenter_add)
{
	if (center_x + xcenter_add > 1 &&
	    center_x + xcenter_add < (int)win->w/2)
		center_x += xcenter_add;
}
			
static void ycenter_add(int ycenter_add)
{
	if (center_y + ycenter_add > 1 &&
	    center_y + ycenter_add < (int)win->h/2)
		center_y += ycenter_add;
}

void event_loop(void)
{
	int shift_pressed;

	GP_Event ev;

	while (GP_EventGet(&ev)) {
		GP_EventDump(&ev);
	
		shift_pressed = GP_EventGetKey(&ev, GP_KEY_LEFT_SHIFT) ||
		                GP_EventGetKey(&ev, GP_KEY_RIGHT_SHIFT);

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
				GP_SWAP(win->w, win->h);
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
				GP_BackendExit(backend);
				exit(0);
			break;
			}
		break;
		case GP_EV_SYS:
			switch(ev.code) {
			case GP_EV_SYS_QUIT:
				GP_BackendExit(backend);
				exit(1);
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
	printf("    1/2/3 ............... choose shape variant (if applicable)\n");
}

int main(int argc, char ** argv)
{
	const char *backend_opts = "X11";

//	GP_SetDebugLevel(10);

	backend_opts = "SDL:320x240";

	backend = GP_BackendInit(backend_opts, "Shapetest", stderr);

	if (backend == NULL) {
		fprintf(stderr, "Failed to initalize backend '%s'\n",
		        backend_opts);
		return 1;
	}
	
	win = backend->context;

	center_x = win->w / 2;
	center_y = win->h / 2;

	/* Load colors compatible with the display */
	black    = GP_ColorToContextPixel(GP_COL_BLACK, win);
	white    = GP_ColorToContextPixel(GP_COL_WHITE, win);
	yellow   = GP_ColorToContextPixel(GP_COL_YELLOW, win);
	green    = GP_ColorToContextPixel(GP_COL_GREEN, win);
	red      = GP_ColorToContextPixel(GP_COL_RED, win);
	gray     = GP_ColorToContextPixel(GP_COL_GRAY_LIGHT, win);
	darkgray = GP_ColorToContextPixel(GP_COL_GRAY_DARK, win);

	print_instructions();
	
	redraw_screen();

	for (;;) {
		GP_BackendWait(backend);
		event_loop();
	}
}
