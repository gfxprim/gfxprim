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

#include <stdio.h>
#include <stdlib.h>
#include <SDL/SDL.h>

#include "GP.h"
#include "GP_SDL.h"

/* The surface used as a display (in fact it is a software surface). */
SDL_Surface *display = NULL;
GP_Context context;

/* Timer used for refreshing the display */
SDL_TimerID timer;

/* An event used for signaling that the timer was triggered. */
SDL_UserEvent timer_event;

Uint32 timer_callback(__attribute__((unused)) Uint32 interval,
			__attribute__((unused)) void * param)
{
	timer_event.type = SDL_USEREVENT;
	SDL_PushEvent((SDL_Event *) &timer_event);
	return 60;
}

/* Basic colors in display-specific format. */
GP_Pixel black, white, yellow, green, red, gray, darkgray;

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
#define SHAPE_LAST	5
static int shape = SHAPE_FIRST;

/* Variants in coordinates, if applicable */
static int variant = 1;

/* Increments added to radii in every timeframe (0 = no change). */
static int xradius_add = 0;
static int yradius_add = 0;

/* center of drawing */
static int center_x = 320;
static int center_y = 240;

static int display_w = 640;
static int display_h = 480;

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
	GP_TPutPixel(&context, x0, y0, green);
	GP_TPutPixel(&context, x1, y1, green);
	GP_TPutPixel(&context, x2, y2, green);

	if (outline == 1)
		GP_TTriangle(&context, x0, y0, x1, y1, x2, y2, yellow);

	if (fill) {
		GP_TFillTriangle(&context, x0, y0, x1, y1, x2, y2, red);
	}

	if (outline == 2)
		GP_TTriangle(&context, x0, y0, x1, y1, x2, y2, white);
}

void draw_testing_circle(int x, int y, int xradius,
			__attribute__((unused)) int yradius)
{
	if (outline == 1)
		GP_TCircle(&context, x, y, xradius, yellow);

	if (fill)
		GP_TFillCircle(&context, x, y, xradius, red);

	if (outline == 2)
		GP_TCircle(&context, x, y, xradius, white);
}

void draw_testing_ring(int x, int y, int xradius,
			__attribute__((unused)) int yradius)
{
	if (outline == 1)
		GP_TCircle(&context, x, y, xradius, yellow);

	if (fill) {
		if (xradius == yradius) {
			GP_TFillRing(&context, x, y, xradius, xradius/2, red);
		} else {
			GP_TFillRing(&context, x, y, GP_MAX(xradius, yradius),
				GP_MIN(xradius, yradius), red);
		}
	}

	if (outline == 2)
		GP_TCircle(&context, x, y, xradius, white);
}

void draw_testing_ellipse(int x, int y, int xradius, int yradius)
{
	if (outline == 1)
		GP_TEllipse(&context, x, y, xradius, yradius, yellow);

	if (fill)
		GP_TFillEllipse(&context, x, y, xradius, yradius, red);

	if (outline == 2)
		GP_TEllipse(&context, x, y, xradius, yradius, white);
}

void draw_testing_rectangle(int x, int y, int xradius, int yradius)
{
	if (outline == 1)
		GP_TRect(&context, x - xradius, y - yradius, x + xradius, y + yradius, yellow);

	if (fill)
		GP_TFillRect(&context, x - xradius, y - yradius, x + xradius, y + yradius, red);

	if (outline == 2)
		GP_TRect(&context, x - xradius, y - yradius, x + xradius, y + yradius, white);
}

void redraw_screen(void)
{

	/* text style for the label */
	GP_TextStyle style = {
		.font = &GP_default_console_font,
		.pixel_xmul = 2,
		.pixel_ymul = 1,
		.pixel_xspace = 0,
		.pixel_yspace = 1,
	};

	SDL_LockSurface(display);
	GP_Fill(&context, black);

	/* axes */
	if (show_axes) {
		GP_THLine(&context, 0, display_w, center_y, gray);
		GP_THLine(&context, 0, display_w, center_y-yradius, darkgray);
		GP_THLine(&context, 0, display_w, center_y+yradius, darkgray);
		GP_TVLine(&context, center_x, 0, display_h, gray);
		GP_TVLine(&context, center_x-xradius, 0, display_h, darkgray);
		GP_TVLine(&context, center_x+xradius, 0, display_h, darkgray);
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
	}
	GP_TText(&context, &style, 16, 16, GP_ALIGN_RIGHT|GP_VALIGN_BELOW, title, white);

	SDL_UnlockSurface(display);
}

void event_loop(void)
{
	static bool shift_pressed = false;
	static int xcenter_add = 0;
	static int ycenter_add = 0;
	SDL_Event event;

	while (SDL_WaitEvent(&event) > 0) {

		switch (event.type) {
		case SDL_USEREVENT:

			if (xradius + xradius_add > 1 && xradius + xradius_add < 400)
				xradius += xradius_add;
			
			if (yradius + yradius_add > 1 && yradius + yradius_add < 400)
				yradius += yradius_add;
			
			if (center_x + xcenter_add > 1 && center_x + xcenter_add < 320)
				center_x += xcenter_add;
			
			if (center_y + ycenter_add > 1 && center_y + ycenter_add < 240)
				center_y += ycenter_add;

			redraw_screen();
			SDL_Flip(display);
			break;

		case SDL_KEYDOWN:
			switch (event.key.keysym.sym) {
			case SDLK_LSHIFT:
			case SDLK_RSHIFT:
				shift_pressed = true;
			break;
			case SDLK_x:
				context.x_swap = !context.x_swap;
			break;
			case SDLK_y:
				context.y_swap = !context.y_swap;
			break;
			case SDLK_r:
				context.axes_swap = !context.axes_swap;

				if (context.axes_swap) {
					display_w = 480;
					display_h = 640;
				} else {
					display_w = 640;
					display_h = 480;
				}
			break;
			case SDLK_f:
				fill = !fill;
				if (!fill && !outline) {
					outline = 1;
				}
				break;

			case SDLK_o:
				outline++;
				if (outline == 3) { outline = 0; }
				if (!fill && outline == 0) {
					fill = 1;
				}
				break;

			case SDLK_a:
				show_axes = !show_axes;
				break;

			case SDLK_ESCAPE:
				return;

			case SDLK_LEFT:
				if (shift_pressed)
					xcenter_add = -1;
				else
					xradius_add = -1;
				break;

			case SDLK_RIGHT:
				if (shift_pressed)
					xcenter_add = 1;
				else
					xradius_add = 1;
				break;

			case SDLK_UP:
				if (shift_pressed)
					ycenter_add = -1;
				else
					yradius_add = 1;
				break;

			case SDLK_DOWN:
				if (shift_pressed)
					ycenter_add = 1;
				else
					yradius_add = -1;
				break;

			case SDLK_PAGEUP:
				xradius_add = 1;
				yradius_add = 1;
				break;

			case SDLK_PAGEDOWN:
				xradius_add = -1;
				yradius_add = -1;
				break;

			case SDLK_1:
				variant = 1;
				break;

			case SDLK_2:
				variant = 2;
				break;

			case SDLK_3:
				variant = 3;
				break;

			case SDLK_4:
				variant = 4;
				break;

			case SDLK_SPACE:
				shape++;
				if (shape > SHAPE_LAST)
					shape = SHAPE_FIRST;
				break;

			case SDLK_EQUALS:
				if (xradius > yradius)
					yradius = xradius;
				else
					xradius = yradius;
				break;

			default:
				break;
			}
			break;
		case SDL_KEYUP:
			switch (event.key.keysym.sym) {
			case SDLK_LSHIFT:
			case SDLK_RSHIFT:
				shift_pressed = false;
			break;
			/* Stop incrementing as soon as the key is released. */
			case SDLK_LEFT:
			case SDLK_RIGHT:
				xradius_add = 0;
				xcenter_add = 0;
				break;
			case SDLK_UP:
			case SDLK_DOWN:
				yradius_add = 0;
				ycenter_add = 0;
				break;

			case SDLK_PAGEUP:
			case SDLK_PAGEDOWN:
				xradius_add = 0;
				yradius_add = 0;
				break;

			default:
				break;
			}
		break;
		case SDL_QUIT:
			return;
		}
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
	int display_bpp = 0;

	int i;
	for (i = 1; i < argc; i++) {
		if (strcmp(argv[i], "-16") == 0) {
			display_bpp = 16;
		}
		else if (strcmp(argv[i], "-24") == 0) {
			display_bpp = 24;
		}
	}

	/* Initialize SDL */
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0) {
		fprintf(stderr, "Could not initialize SDL: %s\n", SDL_GetError());
		return 1;
	}

	/* Create a window with a software back surface */
	display = SDL_SetVideoMode(640, 480, display_bpp, SDL_SWSURFACE);
	if (display == NULL) {
		fprintf(stderr, "Could not open display: %s\n", SDL_GetError());
		goto fail;
	}

	/* Set up a clipping rectangle to exercise clipping */
	SDL_Rect clip_rect = {10, 10, 620, 460};
	SDL_SetClipRect(display, &clip_rect);

	GP_SDL_ContextFromSurface(&context, display);

	/* Load colors compatible with the display */
	GP_ColorNameToPixel(&context, GP_COL_BLACK, &black);
	GP_ColorNameToPixel(&context, GP_COL_WHITE, &white);
	GP_ColorNameToPixel(&context, GP_COL_YELLOW, &yellow);
	GP_ColorNameToPixel(&context, GP_COL_GREEN, &green);
	GP_ColorNameToPixel(&context, GP_COL_RED, &red);
	GP_ColorNameToPixel(&context, GP_COL_GRAY_LIGHT, &gray);
	GP_ColorNameToPixel(&context, GP_COL_GRAY_DARK, &darkgray);

	/* Set up the refresh timer */
	timer = SDL_AddTimer(60, timer_callback, NULL);
	if (timer == 0) {
		fprintf(stderr, "Could not set up timer: %s\n", SDL_GetError());
		goto fail;
	}

	/* Print a short info how to use this test. */
	print_instructions();

	/* Enter the event loop */
	event_loop();

	/* We're done */
	SDL_Quit();
	return 0;

fail:
	SDL_Quit();
	return 1;
}

