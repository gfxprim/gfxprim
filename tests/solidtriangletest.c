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

int pause_drawing = 0;
int draw_outlines = 0;
int draw_vertices = 0;

Uint32 timer_callback(Uint32 interval, void * param)
{
	timer_event.type = SDL_USEREVENT;
	SDL_PushEvent((SDL_Event *) &timer_event);
	return 60;
}

void draw_pixels(void)
{
	if (pause_drawing) return;

	long white = SDL_MapRGB(display->format, 255, 255, 255);
	long pixel;
	int x0 = random() % 320;
	int y0 = random() % 240;
	int x1 = random() % 320;
	int y1 = random() % 240;
	int x2 = random() % 320;
	int y2 = random() % 240;

	SDL_LockSurface(display);
	pixel = SDL_MapRGB(display->format,
				random() % 256,
				random() % 256,
				random() % 256);
	GP_FillTriangle(display, pixel, x0, y0, x1, y1, x2, y2);

	if (draw_vertices) {
		GP_SetPixel(display, white, x0, y0);
		GP_SetPixel(display, white, x1, y1);
		GP_SetPixel(display, white, x2, y2);
	}

	if (draw_outlines) {
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
				draw_pixels();
				SDL_Flip(display);
			break;
			case SDL_KEYDOWN:
				if (event.key.keysym.sym == SDLK_ESCAPE) {
					return;
				}
				else if (event.key.keysym.sym == SDLK_SPACE) {
					pause_drawing = !pause_drawing;
				}
				else if (event.key.keysym.sym == SDLK_o) {
					draw_outlines = !draw_outlines;
				}
				else if (event.key.keysym.sym == SDLK_v) {
					draw_vertices = !draw_vertices;
				}
				break;
			case SDL_QUIT:
				return;
		}
	}
}

int main(int argc, char *argv[])
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

	/* Set up a clipping rectangle to test proper clipping of pixels */
	SDL_Rect clip_rect = { 10, 10, 300, 220 };
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
