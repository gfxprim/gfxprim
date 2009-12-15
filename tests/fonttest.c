#include <stdio.h>
#include <stdlib.h>
#include <SDL/SDL.h>

#include "GP.h"

SDL_Surface * display = NULL;

static long colors[GP_BASIC_COLOR_COUNT];

static const char * test_strings[] = {
	" !\"#$%&\047()*+,-./0123456789:;<=>?@",
	"ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]",
	"abcdefghijklmnopqrstuvwxyz{|}"
};

void redraw_screen(void)
{
	SDL_LockSurface(display);
	
	GP_Clear(display, colors[GP_BLACK]);
	
	GP_TextStyle style = GP_DEFAULT_TEXT_STYLE;
	style.foreground = colors[GP_WHITE];

	const size_t TEST_STRING_COUNT = sizeof(test_strings)/sizeof(const char *);
	size_t i;
	for (i = 0; i < TEST_STRING_COUNT; i++) {
		const char * test_string = test_strings[i];

		style.foreground = colors[GP_WHITE];
		style.pixel_width = 1;
		style.pixel_hspace = 0;
		style.pixel_vspace = 0;

		GP_Text(display, &style, 16, 100*i + 16, test_string);
	
		style.foreground = colors[GP_GRAY];
		style.pixel_width = 2;
		style.pixel_vspace = 1;

		GP_Text(display, &style, 34, 100*i + 34, test_string);

		style.foreground = colors[GP_DARK_GRAY];
		style.pixel_width = 4;
		style.pixel_hspace = 1;
		style.pixel_vspace = 1;

		GP_Text(display, &style, 64, 100*i + 64, test_string);
	}

	SDL_UnlockSurface(display);
}

void event_loop(void)
{
	SDL_Event event;

	while (SDL_WaitEvent(&event) > 0) {
		switch (event.type) {

		case SDL_VIDEOEXPOSE:
			redraw_screen();
			SDL_Flip(display);
			break;

		case SDL_KEYDOWN:
			return;

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

	/* Load a set of basic colors */
	GP_LoadBasicColors(display, colors);

	/* Set up a clipping rectangle to test proper clipping of pixels */
	SDL_Rect clip_rect = { 10, 10, 620, 460 };
	SDL_SetClipRect(display, &clip_rect);

	redraw_screen();
	SDL_Flip(display);

	event_loop();

	SDL_Quit();
	return 0;

fail:
	SDL_Quit();
	return 1;
}
