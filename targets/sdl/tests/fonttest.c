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

/* the display */
SDL_Surface *display = NULL;
GP_Context context;

/* precomputed color pixels in display format */
static GP_Pixel white_pixel, gray_pixel, dark_gray_pixel, black_pixel,
		red_pixel;

static const char *test_strings[] = {
	" !\"#$%&\047()*+,-./0123456789:;<=>?@",
	"ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`",
	"abcdefghijklmnopqrstuvwxyz{|}~",
	"Lorem ipsum dolor sit amet, consectetur adipisicing elit, sed do eiusmod tempor..."
};

/* draw using proportional font? */
static int flag_proportional = 0;

void redraw_screen(void)
{
	SDL_LockSurface(display);
	
	GP_Fill(&context, black_pixel);

	GP_TextStyle style = GP_DEFAULT_TEXT_STYLE;

	if (flag_proportional)
		style.font = &GP_default_proportional_font;
	else
		style.font = &GP_default_console_font;

	/* Text alignment (we are always drawing to the right
	 * and below the starting point).
	 */
	int align = GP_ALIGN_RIGHT|GP_VALIGN_BELOW;

	const size_t TEST_STRING_COUNT = sizeof(test_strings)/sizeof(const char *);
	size_t i;
	for (i = 0; i < TEST_STRING_COUNT; i++) {
		const char * test_string = test_strings[i];

		style.pixel_xmul = 1;
		style.pixel_ymul = 1;
		style.pixel_xspace = 0;
		style.pixel_yspace = 0;

		GP_FillRect(&context,
			16, 100*i + 16,
			16 + GP_TextWidth(&style, test_string),
			100*i + 16 + style.font->height,
			red_pixel);

		GP_Text(&context, &style, 16, 100*i + 16, align, test_string, white_pixel);

		style.pixel_xmul = 2;
		style.pixel_ymul = 2;
		style.pixel_yspace = 1;

		GP_Text(&context, &style, 34, 100*i + 38, align, test_string, gray_pixel);

		style.pixel_xmul = 4;
		style.pixel_ymul = 2;
		style.pixel_xspace = 1;
		style.pixel_yspace = 1;

		GP_Text(&context, &style, 64, 100*i + 72, align, test_string, dark_gray_pixel);
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
			if (event.key.keysym.sym == SDLK_SPACE) {
				flag_proportional = !flag_proportional;
				redraw_screen();
				SDL_Flip(display);
			}
			else if (event.key.keysym.sym == SDLK_ESCAPE) {
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
	display = SDL_SetVideoMode(640, 480, 0, SDL_SWSURFACE);
	if (display == NULL) {
		fprintf(stderr, "Could not open display: %s\n", SDL_GetError());
		goto fail;
	}

	/* Set up a clipping rectangle to test proper clipping of pixels */
	SDL_Rect clip_rect = {10, 10, 620, 460};
	SDL_SetClipRect(display, &clip_rect);

	/* Initialize a GP context from the SDL display */
	GP_SDL_ContextFromSurface(&context, display);

	/* Load colors suitable for the display */
	GP_ColorNameToPixel(&context, GP_COL_WHITE, &white_pixel);
	GP_ColorNameToPixel(&context, GP_COL_GRAY_LIGHT, &gray_pixel);
	GP_ColorNameToPixel(&context, GP_COL_GRAY_DARK, &dark_gray_pixel);
	GP_ColorNameToPixel(&context, GP_COL_BLACK, &black_pixel);
	GP_ColorNameToPixel(&context, GP_COL_RED, &red_pixel);

	redraw_screen();
	SDL_Flip(display);

	event_loop();

	SDL_Quit();
	return 0;

fail:
	SDL_Quit();
	return 1;
}

