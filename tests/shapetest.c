#include <stdio.h>
#include <stdlib.h>
#include <SDL/SDL.h>

#include "GP.h"

/* The surface used as a display (in fact it is a software surface). */
SDL_Surface * display = NULL;

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

/* Colors */
long white, red, blue, gray, darkgray, black;

void init_colors(SDL_Surface * surf)
{
	white = SDL_MapRGB(surf->format, 255, 255, 255);
	red = SDL_MapRGB(surf->format, 255, 0, 0);
	blue = SDL_MapRGB(surf->format, 0, 0, 255);
	gray = SDL_MapRGB(surf->format, 127, 127, 127);
	darkgray = SDL_MapRGB(surf->format, 63, 63, 63);
	black = SDL_MapRGB(surf->format, 0, 0, 0);
}

/* Radius of the shape being drawn */
static int xradius = 5;
static int yradius = 5;

/* Draw outline? */
static int outline = 0;

/* Fill the shape? */
static int fill = 1;

/* Show axes? */
static int show_axes = 1;

/* Shape to be drawn */
#define SHAPE_FIRST	1
#define SHAPE_TRIANGLE	1
#define SHAPE_CIRCLE    2
#define SHAPE_ELLIPSE	3
#define SHAPE_RECTANGLE	4
#define SHAPE_LAST	4
static int shape = SHAPE_FIRST;

/* Variants in coordinates, if applicable */
static int variant = 1;

/* Increments added to radii in every timeframe (0 = no change). */
static int xradius_add = 0;
static int yradius_add = 0;

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
	default:
		x0 = x;
		y0 = y - yradius;
		x1 = x + xradius;
		y1 = y + yradius;
		x2 = x - xradius;
		y2 = y + yradius;
		break;
	}

	if (fill) {
		GP_FillTriangle(display, red, x0, y0, x1, y1, x2, y2);
	}

	if (outline) {
		GP_Triangle(display, white, x0, y0, x1, y1, x2, y2);
	}
}

void draw_testing_circle(int x, int y, int xradius,
			__attribute__((unused)) int yradius)
{
	if (fill) {
		GP_FillCircle(display, red, x, y, xradius);
	}
	if (outline) {
		GP_Circle(display, white, x, y, xradius);
	}
}

void draw_testing_ellipse(int x, int y, int xradius, int yradius)
{
	if (fill) {
		GP_FillEllipse(display, red, x, y, xradius, yradius);
	}
	if (outline) {
		GP_Ellipse(display, white, x, y, xradius, yradius);
	}
}

void draw_testing_rectangle(int x, int y, int xradius, int yradius)
{
	if (fill) {
		GP_FillRect(display, red, x - xradius, y - yradius, x + xradius, y + yradius);
	}
	if (outline) {
		GP_Rect(display, white, x - xradius, y - yradius, x + xradius, y + yradius);
	}
}

void redraw_screen(void)
{
	int x = 320;
	int y = 240;

	SDL_LockSurface(display);
	GP_FillRect(display, black, 0, 0, 640, 480);

	/* axes */
	if (show_axes) {
		GP_HLine(display, gray, 0, 640, y);
		GP_HLine(display, darkgray, 0, 640, y-yradius);
		GP_HLine(display, darkgray, 0, 640, y+yradius);
		GP_VLine(display, gray, x, 0, 480);
		GP_VLine(display, darkgray, x-xradius, 0, 480);
		GP_VLine(display, darkgray, x+xradius, 0, 480);
	}

	/* the shape */
	switch (shape) {
	case SHAPE_TRIANGLE:
		draw_testing_triangle(x, y, xradius, yradius);
		break;
	case SHAPE_CIRCLE:
		draw_testing_circle(x, y, xradius, yradius);
		break;
	case SHAPE_ELLIPSE:
		draw_testing_ellipse(x, y, xradius, yradius);
		break;
	case SHAPE_RECTANGLE:
		draw_testing_rectangle(x, y, xradius, yradius);
		break;
	}

	SDL_UnlockSurface(display);
}

void event_loop(void)
{
	SDL_Event event;

	while (SDL_WaitEvent(&event) > 0) {

		switch (event.type) {
		case SDL_USEREVENT:

			if (xradius + xradius_add > 1 && xradius + xradius_add < 400)
				xradius += xradius_add;
			if (yradius + yradius_add > 1 && yradius + yradius_add < 400)
				yradius += yradius_add;
			
			redraw_screen();
			SDL_Flip(display);
			break;

		case SDL_KEYDOWN:
			switch (event.key.keysym.sym) {

			case SDLK_f:
				fill = !fill;
				if (!fill && !outline) {
					outline = 1;
				}
				break;

			case SDLK_o:
				outline = !outline;
				if (!fill && !outline) {
					fill = 1;
				}
				break;

			case SDLK_x:
				show_axes = !show_axes;
				break;

			case SDLK_ESCAPE:
				return;

			case SDLK_LEFT:
				xradius_add = -1;
				break;

			case SDLK_RIGHT:
				xradius_add = 1;
				break;

			case SDLK_UP:
				yradius_add = 1;
				break;

			case SDLK_DOWN:
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

			/* Stop incrementing as soon as the key is released. */
			case SDLK_LEFT:
			case SDLK_RIGHT:
				xradius_add = 0;
				break;
			case SDLK_UP:
			case SDLK_DOWN:
				yradius_add = 0;
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
	printf("    Esc ......... exit\n");
	printf("    Space ....... change shapes\n");
	printf("    O ........... toggle outline drawing\n");
	printf("    F ........... toggle filling\n");
	printf("    X ........... show/hide axes\n");
	printf("    left/right .. increase/decrease horizontal radius\n");
	printf("    up/down ..... increase/decrease vertical radius\n");
	printf("    PgUp/PgDn ... increase/decrease both radii\n");
	printf("    = ........... reset radii to the same value\n");
	printf("    1/2/3 ....... choose shape variant (if applicable)\n");
}

int main(void)
{
	/* Initialize SDL */
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0) {
		fprintf(stderr, "Could not initialize SDL: %s\n", SDL_GetError());
		return 1;
	}

	/* Create a window with a software back surface */
	display = SDL_SetVideoMode(640, 480, 0, SDL_SWSURFACE);
	if (display == NULL) {
		fprintf(stderr, "Could not open display: %s\n", SDL_GetError());
		goto fail;
	}

	/* Find pixel representations of needed colors */
	init_colors(display);

	/* Set up a clipping rectangle to exercise clipping */
	SDL_Rect clip_rect = { 10, 10, 620, 460 };
	SDL_SetClipRect(display, &clip_rect);

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
