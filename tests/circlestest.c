#include <stdio.h>
#include <stdlib.h>
#include <SDL/SDL.h>

#include "GP.h"

SDL_Surface * display = NULL;

static int state;

void draw1(void)
{
	long pixel;
	int x = 160;
	int y = 120;
	int r;

	SDL_LockSurface(display);
	
	for (r = 202; r > 0; r -= 3) {
		pixel = SDL_MapRGB(display->format, 150*(r%2) + r/2, 0, 0);
		GP_FillCircle(display, pixel, x, y, r);
	}
	
	
	SDL_UnlockSurface(display);
}

void draw2(void)
{
	long pixel;
	int x = 160;
	int y = 120;
	int r;

	SDL_LockSurface(display);
	
	for (r = 203; r > 0; r -= 3) {
		pixel = SDL_MapRGB(display->format, 0, 150*(r%2) + r/2, 0);
		GP_FillCircle(display, pixel, x, y, r);
	}
	
	
	SDL_UnlockSurface(display);
}

void draw3(void)
{
	long pixel;
	int x = 160;
	int y = 120;
	int r;

	SDL_LockSurface(display);
	
	for (r = 201; r > 0; r -= 3) {
		pixel = SDL_MapRGB(display->format, 0, 0, 150*(r%2) + r/2);
		GP_FillCircle(display, pixel, x, y, r);
	}
	
	
	SDL_UnlockSurface(display);
}

void draw4(void)
{
	long pixel;
	int x = 160;
	int y = 120;
	int r;

	SDL_LockSurface(display);
	
	SDL_FillRect(display, NULL, 0xff000000);
	
	for (r = 202; r > 0; r -= 3) {
		pixel = SDL_MapRGB(display->format, 150 + r/2, 0, 0);
		GP_Circle(display, pixel, x, y, r);
	}
	
	
	SDL_UnlockSurface(display);
}

void draw5(void)
{
	long pixel;
	int x = 160;
	int y = 120;
	int r;

	SDL_LockSurface(display);
	
	SDL_FillRect(display, NULL, 0xff000000);
	
	for (r = 203; r > 0; r -= 3) {
		pixel = SDL_MapRGB(display->format, 0, 150 + r/2, 0);
		GP_Circle(display, pixel, x, y, r);
	}
	
	
	SDL_UnlockSurface(display);
}

void draw6(void)
{
	long pixel;
	int x = 160;
	int y = 120;
	int r;

	SDL_LockSurface(display);
	
	SDL_FillRect(display, NULL, 0xff000000);
	
	for (r = 201; r > 0; r -= 3) {
		pixel = SDL_MapRGB(display->format, 0, 0, 150 + r/2);
		GP_Circle(display, pixel, x, y, r);
	}
	
	
	SDL_UnlockSurface(display);
}

void event_loop(void)
{
	SDL_Event event;

	while (SDL_WaitEvent(&event) > 0) {

		switch (event.type) {
			case SDL_KEYDOWN:
				switch (state++) {
					case 0:
						draw2();
						SDL_Flip(display);
					break;
					case 1:
						draw3();
						SDL_Flip(display);
					break;
					case 2:
						draw4();
						SDL_Flip(display);
					break;
					case 3:
						draw5();
						SDL_Flip(display);
					break;
					case 4:
						draw6();
						SDL_Flip(display);
					break;
					default:
						return;
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
	display = SDL_SetVideoMode(320, 240, 0, SDL_SWSURFACE);
	if (display == NULL) {
		fprintf(stderr, "Could not open display: %s\n", SDL_GetError());
		goto fail;
	}

	/* Set up a clipping rectangle to test proper clipping of pixels */
	SDL_Rect clip_rect = { 10, 10, 300, 220 };
	SDL_SetClipRect(display, &clip_rect);

	draw1();
	SDL_Flip(display);

	event_loop();

	SDL_Quit();
	return 0;

fail:
	SDL_Quit();
	return 1;
}
