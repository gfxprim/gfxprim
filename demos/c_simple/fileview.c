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
 * Copyright (C) 2009-2013 Cyril Hrubis <metan@ucw.cz>                       *
 *                                                                           *
 *****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <GP.h>

static GP_Context *win;
static GP_Backend *backend;

static GP_Pixel white_pixel, gray_pixel, dark_gray_pixel, black_pixel,
		red_pixel, blue_pixel;

static int font_flag = 0;
static int tracking = 0;

static int mul = 1;
static int space = 0;

static GP_FontFace *font;

struct FileLine {
	char *text;
	struct FileLine *next;
	struct FileLine *prev;
};

struct FileLine *first_line = NULL;
struct FileLine *last_line = NULL;

void redraw_screen(void)
{
	GP_Fill(win, gray_pixel);

	GP_TextStyle style = GP_DEFAULT_TEXT_STYLE;

	switch (font_flag) {
	case 0:
		style.font = &GP_DefaultConsoleFont;
	break;
	case 1:
		style.font = &GP_DefaultProportionalFont;
	break;
	case 2:
		style.font = GP_FontTinyMono;
	break;
	case 3:
		style.font = GP_FontTiny;
	break;
	case 4:
		style.font = GP_FontC64;
	break;
	case 5:
		style.font = font;
	break;
	}

	style.pixel_xmul = mul;
	style.pixel_ymul = mul;
	style.pixel_xspace = space;
	style.pixel_yspace = space;
	style.char_xspace = tracking;

	/* Text alignment (we are always drawing to the right
	 * and below the starting point).
	 */
	int align = GP_ALIGN_RIGHT|GP_VALIGN_BELOW;

	struct FileLine *line = first_line;
	unsigned int i;
	for (i = 0; i < win->h/GP_TextHeight(&style); i++) {
		if (line == NULL)
			break;
		GP_Text(win, &style, 16, 16 + (1.0 * GP_TextHeight(&style))*i,
		        align, black_pixel, gray_pixel, line->text);
		line = line->next;
	}
}

static void warp_up(int lines)
{
	while (lines-- > 0)
		if (first_line->prev != NULL)
			first_line = first_line->prev;

	redraw_screen();
	GP_BackendFlip(backend);
}

static void warp_down(int lines)
{
	while (lines-- > 0)
		if (first_line->next != NULL)
			first_line = first_line->next;

	redraw_screen();
	GP_BackendFlip(backend);
}

void event_loop(void)
{
	GP_Event ev;

	for (;;) {
		GP_BackendWaitEvent(backend, &ev);

		switch (ev.type) {
		case GP_EV_KEY:
			if (ev.code != GP_EV_KEY_DOWN)
				continue;

			switch (ev.val.key.key) {
			case GP_KEY_SPACE:
				if (font)
					font_flag = (font_flag + 1) % 6;
				else
					font_flag = (font_flag + 1) % 5;

				redraw_screen();
				GP_BackendFlip(backend);
			break;
			case GP_KEY_RIGHT:
				tracking++;
				redraw_screen();
				GP_BackendFlip(backend);
			break;
			case GP_KEY_LEFT:
				tracking--;
				redraw_screen();
				GP_BackendFlip(backend);
			break;
			case GP_KEY_UP:
				warp_up(1);
			break;
			case GP_KEY_DOWN:
				warp_down(1);
			break;
			case GP_KEY_DOT:
				space++;
				redraw_screen();
				GP_BackendFlip(backend);
			break;
			case GP_KEY_COMMA:
				space--;
				redraw_screen();
				GP_BackendFlip(backend);
			break;
			case GP_KEY_RIGHT_BRACE:
				mul++;
				redraw_screen();
				GP_BackendFlip(backend);
			break;
			case GP_KEY_LEFT_BRACE:
				if (mul > 0)
					mul--;
				redraw_screen();
				GP_BackendFlip(backend);
			break;
			case GP_KEY_PAGE_UP:
				warp_up(30);
			break;
			case GP_KEY_PAGE_DOWN:
				warp_down(30);
			break;
			case GP_KEY_ESC:
				GP_BackendExit(backend);
				exit(0);
			break;
			}
		break;
		case GP_EV_SYS:
			switch(ev.code) {
			case GP_EV_SYS_QUIT:
				GP_BackendExit(backend);
				exit(0);
			break;
			}
		break;
		}
	}
}

static int read_file_head(const char *filename)
{
	FILE *f = fopen(filename, "r");
	char buf[512];

	if (f == NULL) {
		fprintf(stderr, "Could not open file: %s\n", filename);
		return 0;
	}

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
	const char *backend_opts = "X11";

	if (argc == 1) {
		fprintf(stderr, "No file specified\n");
		return 1;
	}

	if (argc > 2)
		font = GP_FontFaceLoad(argv[2], 0, 16);

	if (!read_file_head(argv[1]))
		return 1;

	backend = GP_BackendInit(backend_opts, "File View");

	if (backend == NULL) {
		fprintf(stderr, "Failed to initalize backend '%s'\n",
		        backend_opts);
		return 1;
	}

	win = backend->context;

	white_pixel     = GP_ColorToContextPixel(GP_COL_WHITE, win);
	gray_pixel      = GP_ColorToContextPixel(GP_COL_GRAY_LIGHT, win);
	dark_gray_pixel = GP_ColorToContextPixel(GP_COL_GRAY_DARK, win);
	black_pixel     = GP_ColorToContextPixel(GP_COL_BLACK, win);
	red_pixel       = GP_ColorToContextPixel(GP_COL_RED, win);
	blue_pixel      = GP_ColorToContextPixel(GP_COL_BLUE, win);

	redraw_screen();
	GP_BackendFlip(backend);

	event_loop();

	return 0;
}
