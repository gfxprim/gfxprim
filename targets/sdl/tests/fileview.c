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
		red_pixel, blue_pixel;

/* draw using proportional font? */
static int font_flag = 0;

/* font tracking */
static int tracking = 0;

/* font to be used */
GP_Font *font;

struct FileLine {
	char *text;		/* null-terminated, malloc'd string */
	struct FileLine *next;	/* next line or NULL for the last line */
	struct FileLine *prev;
};

struct FileLine *first_line = NULL;
struct FileLine *last_line = NULL;

void redraw_screen(void)
{
	SDL_LockSurface(display);
	
	GP_Fill(&context, gray_pixel);

	GP_TextStyle style = GP_DEFAULT_TEXT_STYLE;

	switch (font_flag) {
	case 0:
		style.font = &GP_default_console_font;
	break;
	case 1:
		style.font = &GP_default_proportional_font;
	break;
	case 2:
		style.font = font;
	break;
	}

	style.pixel_xmul = 1;
	style.pixel_ymul = 1;
	style.pixel_xspace = 0;
	style.pixel_yspace = 0;
	style.char_xspace = tracking;

	/* Text alignment (we are always drawing to the right
	 * and below the starting point).
	 */
	int align = GP_ALIGN_RIGHT|GP_VALIGN_BELOW;

	struct FileLine *line = first_line;
	unsigned int i;
	for (i = 0; i < 30; i++) { 
		if (line == NULL)
			break;
		GP_Text(&context, &style, 16, 16*i + 16, align, line->text, black_pixel);
		line = line->next;
	}

	SDL_UnlockSurface(display);
}

static void warp_up(int lines)
{
	while (lines-- > 0)
		if (first_line->prev != NULL)
			first_line = first_line->prev;
	
	redraw_screen();
	SDL_Flip(display);
}

static void warp_down(int lines)
{
	while (lines-- > 0)
		if (first_line->next != NULL)
			first_line = first_line->next;
	
	redraw_screen();
	SDL_Flip(display);
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
			switch (event.key.keysym.sym) {
			case SDLK_SPACE:
				if (font)
					font_flag = (font_flag + 1) % 3;
				else
					font_flag = (font_flag + 1) % 2;
					
				redraw_screen();
				SDL_Flip(display);
			break;
			case SDLK_RIGHT:
				tracking++;
				redraw_screen();
				SDL_Flip(display);
			break;
			case SDLK_LEFT:
				tracking--;
				redraw_screen();
				SDL_Flip(display);
			break;
			case SDLK_ESCAPE:
				return;
			case SDLK_UP:
				warp_up(1);
			break;
			case SDLK_PAGEUP:
				warp_up(29);
			break;
			case SDLK_DOWN:
				warp_down(1);
			break;
			case SDLK_PAGEDOWN:
				warp_down(29);
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

static int read_file_head(const char *filename)
{
	FILE *f = fopen(filename, "r");
	if (f == NULL) {
		fprintf(stderr, "Could not open file: %s\n", filename);
		return 0;
	}

	char buf[512];
	for (;;) {

		if (fgets(buf, 511, f) == NULL)
			break;
		
		struct FileLine *line = malloc(sizeof(*line));
		line->text = strdup(buf);
		line->next = NULL;
		line->prev = NULL;

		if (first_line == NULL) {
			first_line = line;
			last_line = line;
		} else {
			line->prev = last_line;
			last_line->next = line;
			last_line = line;
		}
	}

	fclose(f);
	return 1;
}

int main(int argc, char *argv[])
{
	if (argc == 1) {
		fprintf(stderr, "No file specified\n");
		return 1;
	}

	if (!read_file_head(argv[1]))
		return 1;

	/* Initialize SDL */
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0) {
		fprintf(stderr, "Could not initialize SDL: %s\n", SDL_GetError());
		return 1;
	}

	/* Create a window with a software back surface */
	display = SDL_SetVideoMode(800, 480, 0, SDL_SWSURFACE);
	if (display == NULL) {
		fprintf(stderr, "Could not open display: %s\n", SDL_GetError());
		goto fail;
	}

	/* Initialize a GP context from the SDL display */
	GP_SDL_ContextFromSurface(&context, display);

	/* Load colors suitable for the display */
	GP_ColorNameToPixel(&context, GP_COL_WHITE, &white_pixel);
	GP_ColorNameToPixel(&context, GP_COL_GRAY_LIGHT, &gray_pixel);
	GP_ColorNameToPixel(&context, GP_COL_GRAY_DARK, &dark_gray_pixel);
	GP_ColorNameToPixel(&context, GP_COL_BLACK, &black_pixel);
	GP_ColorNameToPixel(&context, GP_COL_RED, &red_pixel);
	GP_ColorNameToPixel(&context, GP_COL_BLUE, &blue_pixel);

	redraw_screen();
	SDL_Flip(display);

	event_loop();

	SDL_Quit();
	return 0;

fail:
	SDL_Quit();
	return 1;
}

