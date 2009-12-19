#include <math.h>
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

/* Values for color pixels in display format. */
static long colors[GP_BASIC_COLOR_COUNT];

Uint32 timer_callback(__attribute__((unused)) Uint32 interval,
			__attribute__((unused)) void * param)
{
	timer_event.type = SDL_USEREVENT;
	SDL_PushEvent((SDL_Event *) &timer_event);
	return 30;
}

double start_angle = 0.0;

void redraw_screen(void)
{
	int i;

	for (i = 1; i < 10; i++) {
		GP_HLineWide(display, colors[GP_WHITE], GP_LINE_CENTER, i, 10, 100, 20*i);
		GP_HLine(display, colors[GP_RED], 10, 100, 20*i);
	}

	for (i = 1; i < 10; i++) {
		GP_VLineWide(display, colors[GP_WHITE], GP_LINE_CENTER, i, 100 + 20*i, 100, 190);
		GP_VLine(display, colors[GP_RED], 100 + 20*i, 100, 190);
	}
}

void event_loop(void)
{
	SDL_Event event;

        while (SDL_WaitEvent(&event) > 0) {
		switch (event.type) {
			case SDL_USEREVENT:
                		redraw_screen();
				SDL_Flip(display);
				start_angle += 0.01;
				if (start_angle > 2*M_PI) {
					start_angle = 0.0;
				}
            		break;
			case SDL_KEYDOWN:
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

	/* Print basic information about the surface */
	printf("Display surface properties:\n");
	printf("    width: %4d, height: %4d, pitch: %4d\n",
	       display->w, display->h, display->pitch);
	printf("    bits per pixel: %2d, bytes per pixel: %2d\n",
	       display->format->BitsPerPixel, display->format->BytesPerPixel);

	/* Get colors */
	GP_LoadBasicColors(display, colors);

	/* Set up a clipping rectangle to test proper clipping of pixels */
	SDL_Rect clip_rect = { 10, 10, 620, 460 };
	SDL_SetClipRect(display, &clip_rect);

	/* Set up the refresh timer */
	timer = SDL_AddTimer(30, timer_callback, NULL);
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

