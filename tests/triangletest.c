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

Uint32 timer_callback(Uint32 interval, void * param)
{
	timer_event.type = SDL_USEREVENT;
	SDL_PushEvent((SDL_Event *) &timer_event);
	return 60;
}

static int radius = 1;
static int outline = 0;
static int shape = 1;
static int radius_add = 0;

void draw_pixels(void)
{
	int x = 320;
	int y = 240;
	long white = SDL_MapRGB(display->format, 255, 255, 255);
	long blue = SDL_MapRGB(display->format, 0, 0, 255);
	long gray = SDL_MapRGB(display->format, 127, 127, 127);
	long darkgray = SDL_MapRGB(display->format, 63, 63, 63);
	long black = SDL_MapRGB(display->format, 0, 0, 0);

	SDL_LockSurface(display);
	GP_FillRect(display, black, 0, 0, 640, 480);

	/* axes */
	GP_HLine(display, gray, 0, 640, y);
	GP_HLine(display, darkgray, 0, 640, y-radius);
	GP_HLine(display, darkgray, 0, 640, y+radius);
	GP_VLine(display, gray, x, 0, 480);
	GP_VLine(display, darkgray, x-radius, 0, 480);
	GP_VLine(display, darkgray, x+radius, 0, 480);

	/* the shape */
	int x0, y0, x1, y1, x2, y2;
	switch (shape) {
	case 1:
		x0 = x;
		y0 = y - radius;
		x1 = x - radius;
		y1 = y;
		x2 = x + radius;
		y2 = y + radius;
		break;
	case 2:
		x0 = x - radius;
		y0 = y - radius;
		x1 = x + radius;
		y1 = y;
		x2 = x + radius;
		y2 = y + radius;
		break;
	
	case 3:
		x0 = x;
		y0 = y - radius;
		x1 = x + radius;
		y1 = y + radius;
		x2 = x - radius;
		y2 = y + radius;
		break;
	}

	GP_FillTriangle(display, blue, x0, y0, x1, y1, x2, y2);

	if (outline) {
		GP_Triangle(display, white, x0, y0, x1, y1, x2, y2);
	}

	SDL_UnlockSurface(display);
}

void event_loop(void)
{
	SDL_Event event;

	while (SDL_WaitEvent(&event) > 0) {

		switch (event.type) {
		case SDL_USEREVENT:
			if (radius + radius_add > 1 || radius + radius_add < 400)
				radius += radius_add;
			
			draw_pixels();
			SDL_Flip(display);
			break;
		case SDL_KEYDOWN:
			switch (event.key.keysym.sym) {
			case SDLK_o:
				outline = !outline;
				break;

			case SDLK_ESCAPE:
				return;

			case SDLK_LEFT:
				radius_add = -1;
				break;

			case SDLK_RIGHT:
				radius_add = 1;
				break;

			case SDLK_1:
				shape = 1;
				break;

			case SDLK_2:
				shape = 2;
				break;

			case SDLK_3:
				shape = 3;
				break;
			default:
				break;
			}
			break;
		case SDL_KEYUP:
			switch (event.key.keysym.sym) {
			case SDLK_LEFT:
			case SDLK_RIGHT:
				radius_add = 0;
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

	/* Set up a clipping rectangle to test proper clipping of pixels */
	SDL_Rect clip_rect = { 10, 10, 630, 470 };
	SDL_SetClipRect(display, &clip_rect);

	/* Set up the refresh timer */
	timer = SDL_AddTimer(60, timer_callback, NULL);
	if (timer == 0) {
		fprintf(stderr, "Could not set up timer: %s\n", SDL_GetError());
		goto fail;
	}

	/* Enter the event loop */
	event_loop();

	/* We're done */
	SDL_Quit();
	return 0;

fail:
	SDL_Quit();
	return 1;
}
