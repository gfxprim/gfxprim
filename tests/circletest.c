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
static int radius_add = 0;

void draw_pixels(void)
{
	int x = 320;
	int y = 240;
	SDL_Rect display_rect = { 0, 0, 640, 480 };
	long black = SDL_MapRGB(display->format, 0, 0, 0);
	long white = SDL_MapRGB(display->format, 255, 255, 255);
	long red = SDL_MapRGB(display->format, 255, 0, 0);
	long gray = SDL_MapRGB(display->format, 127, 127, 127);
	long darkgray = SDL_MapRGB(display->format, 63, 63, 63);

	SDL_LockSurface(display);
	SDL_FillRect(display, &display_rect, black);

	/* axes */
	GP_HLine(display, gray, 0, 640, y);
	GP_HLine(display, darkgray, 0, 640, y-radius);
	GP_HLine(display, darkgray, 0, 640, y+radius);
	GP_VLine(display, gray, x, 0, 480);
	GP_VLine(display, darkgray, x-radius, 0, 480);
	GP_VLine(display, darkgray, x+radius, 0, 480);

	/* The filled circle. */
	GP_FillCircle(display, red, x, y, radius);

	/* White circle to show underdrawing errors in FillCircle(). */
	if (outline && radius > 4) {
		GP_Circle(display, white, x, y, radius);
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
			case SDLK_LEFT:
				radius_add = -1;
				break;
			case SDLK_RIGHT:
				radius_add = 1;
				break;
			case SDLK_o:
				outline = !outline;
				break;
			case SDLK_ESCAPE:
				return;
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
		default:
			break;
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
