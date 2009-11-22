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
long black, red, green, blue, yellow, white;

Uint32 timer_callback(Uint32 interval, void * param)
{
	timer_event.type = SDL_USEREVENT;
	SDL_PushEvent((SDL_Event *) &timer_event);
	return 30;
}

double start_angle = 0.0;

void draw_lines(void)
{
	double angle;
	int x, y;

	SDL_LockSurface(display);

	GP_Clear(display, black);

	/* axes */
	GP_Line(display, white, 0, 120, 320, 120);
	GP_Line(display, white, 160, 0, 160, 240);

	for (angle = 0.0; angle < 2*M_PI; angle += 0.1) {
		x = (int) (120.0 * cos(start_angle + angle));
		y = (int) (90.0 * sin(start_angle + angle));

		Uint8 r = 127.0 + 127.0 * cos(start_angle + angle);
		Uint8 g = 127.0 + 127.0 * sin(start_angle + angle);
		
		Uint32 color = SDL_MapRGB(display->format, r, 0, g);
		
		GP_SetPixel(display, white, 160 + x, 120 + y);
		GP_Line(display, color, 160, 120, 160 + x, 120 + y);
		GP_Line(display, color, 160 + x, 120 + y, 160, 120);
	}

	SDL_UnlockSurface(display);
}

void event_loop(void)
{
	SDL_Event event;

        while (SDL_WaitEvent(&event) > 0) {
		switch (event.type) {
			case SDL_USEREVENT:
                		draw_lines();
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
	black = SDL_MapRGB(display->format, 0, 0, 0);
	white = SDL_MapRGB(display->format, 255, 255, 255);
	red = SDL_MapRGB(display->format, 255, 0, 0);
	green = SDL_MapRGB(display->format, 0, 255, 0);
	blue = SDL_MapRGB(display->format, 0, 0, 255);
	yellow = SDL_MapRGB(display->format, 255, 255, 0);

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

