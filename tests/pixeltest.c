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
long red;
long green;
long blue;

Uint32 timer_callback(Uint32 interval, void * param)
{
	timer_event.type = SDL_USEREVENT;
	SDL_PushEvent((SDL_Event *) &timer_event);
	return 30;
}

void draw_pixels(void)
{
	long pixel;
	int x = random() % 320;
	int y = random() % 240;

	SDL_LockSurface(display);
	pixel = GP_GetPixel(display, x, y);
    
	if (pixel == green)
		GP_SetPixel(display, blue, x, y);
	else
		GP_SetPixel(display, green, x, y);

	SDL_UnlockSurface(display);
}

void event_loop(void)
{
	SDL_Event event;

	while (SDL_WaitEvent(&event) > 0) {

		switch (event.type) {
			case SDL_USEREVENT:
				draw_pixels();
				SDL_Flip(display);
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
	display = SDL_SetVideoMode(320, 240, 0, SDL_SWSURFACE);
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
	red = SDL_MapRGB(display->format, 255, 0, 0);
	green = SDL_MapRGB(display->format, 0, 255, 0);
	blue = SDL_MapRGB(display->format, 0, 0, 255);

	/* Set up a clipping rectangle to test proper clipping of pixels */
	SDL_Rect clip_rect = { 10, 10, 300, 220 };
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
