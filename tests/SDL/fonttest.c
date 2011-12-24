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
 * Copyright (C) 2009-2011 Cyril Hrubis <metan@ucw.cz>                       *
 *                                                                           *
 *****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <SDL/SDL.h>

#include "GP.h"
#include "GP_SDL.h"

SDL_Surface *display = NULL;
GP_Context context;

static GP_Pixel white_pixel, gray_pixel, dark_gray_pixel, black_pixel,
		red_pixel, blue_pixel;

static const char *test_strings[] = {
	" !\"#$%&\047()*+,-./0123456789:;<=>?@",
	"ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`",
	"abcdefghijklmnopqrstuvwxyz{|}~",
	"Lorem ipsum dolor sit amet, consectetur adipisicing elit, sed do eiusmod tempor..."
};

static int font_flag = 0;
static int tracking = 0;
GP_FontFace *font = NULL;

static const char *glyph_bitmap_format_name(const GP_FontBitmapFormat format)
{
	switch (format) {
	case GP_FONT_BITMAP_1BPP:
		return "1BPP";
	break;
	case GP_FONT_BITMAP_8BPP:
		return "8BPP";
	break;
	default:
		return "Unknown";
	}
}

static void print_character_metadata(const GP_FontFace *font, int c)
{
	const GP_GlyphBitmap *glyph = GP_GetGlyphBitmap(font, c);
	fprintf(stderr, "Properties of the character '%c':\n", c);

	if (glyph) {
		fprintf(stderr, "   bitmap width: %d, height: %d\n",
				glyph->width, glyph->height);
		fprintf(stderr, "          bearing_x: %d bearing_y %d\n",
		                glyph->bearing_x, glyph->bearing_y);
		fprintf(stderr, "          advance_x: %d\n",
		                glyph->advance_x);
	} else {
		fprintf(stderr, "(null)\n");
	}
}

static void print_font_properties(const GP_FontFace *font)
{
	fprintf(stderr, "Font properties:\n");
	fprintf(stderr, "    Height: ascend: %d, descend: %d\n",
			GP_FontAscend(font), GP_FontDescend(font));
	fprintf(stderr, "    Glyph bitmap format: %s\n", 
	                glyph_bitmap_format_name(font->glyph_bitmap_format));
	fprintf(stderr, "    Bounding box width: %d, heigth: %d\n",
	                GP_FontMaxWidth(font), GP_FontHeight(font));

	print_character_metadata(font, 'a');
	print_character_metadata(font, 'm');
	print_character_metadata(font, '0');
}

#define SPACING 120

void redraw_screen(void)
{
	SDL_LockSurface(display);
	
	GP_Fill(&context, black_pixel);

	GP_TextStyle style = GP_DEFAULT_TEXT_STYLE;

	switch (font_flag) {
	case 0:
		style.font = &GP_DefaultProportionalFont;
	break;
	case 1:
		style.font = &GP_DefaultConsoleFont;
	break;
	case 2:
		style.font = font;
	break;
	}

	print_font_properties(style.font);

	/* Text alignment (we are always drawing to the right
	 * and below the starting point).
	 */
	int align = GP_ALIGN_RIGHT|GP_VALIGN_BELOW;

	const size_t TEST_STRING_COUNT = sizeof(test_strings)/sizeof(const char *);
	size_t i;
	for (i = 0; i < TEST_STRING_COUNT; i++) {
		const char *test_string = test_strings[i];

		style.pixel_xmul = 1;
		style.pixel_ymul = 1;
		style.pixel_xspace = 0;
		style.pixel_yspace = 0;
		style.char_xspace = tracking;

		GP_FillRectXYWH(&context,
			16, SPACING*i + 16,
			GP_TextWidth(&style, test_string),
			GP_FontHeight(style.font),
			dark_gray_pixel);

		GP_RectXYWH(&context,
			15, SPACING*i + 15,
			GP_TextMaxWidth(&style, strlen(test_string)) + 1,
			GP_FontHeight(style.font) + 1,
			blue_pixel);

		GP_Text(&context, &style, 16, SPACING*i + 16, align,
		        white_pixel, dark_gray_pixel, test_string);
		
		style.pixel_xmul = 2;
		style.pixel_ymul = 2;
		style.pixel_yspace = 1;

		GP_Text(&context, &style, 34, SPACING*i + 44, align,
		        white_pixel, black_pixel, test_string);

		style.pixel_xmul = 4;
		style.pixel_ymul = 2;
		style.pixel_xspace = 1;
		style.pixel_yspace = 1;

		GP_Text(&context, &style, 64, SPACING*i + 88, align,
		        dark_gray_pixel, black_pixel, test_string);
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
			switch (event.key.keysym.sym) {
			case SDLK_SPACE:
				if (font)
					font_flag = (font_flag + 1) % 3;
				else
					font_flag = (font_flag + 1) % 2;
					
				redraw_screen();
				SDL_Flip(display);
			break;
			case SDLK_UP:
				tracking++;
				redraw_screen();
				SDL_Flip(display);
			break;
			case SDLK_DOWN:
				tracking--;
				redraw_screen();
				SDL_Flip(display);
			break;
			case SDLK_ESCAPE:
				return;
			default:
			break;
			}
		break;

		case SDL_QUIT:
			return;
		}
	}
}

void print_instructions(void)
{
	printf("Use the following keys to control the test:\n");
	printf("    Esc ................. exit\n");
	printf("    Space ............... change font\n");
	printf("    up/down ............. increase/decrease tracking\n");
}

int main(int argc, char *argv[])
{
	print_instructions();

	GP_SetDebugLevel(10);

	if (argc > 1) {
		fprintf(stderr, "\nLoading font '%s'\n", argv[1]);
		font = GP_FontFaceLoad(argv[1], 12, 16);
	}

	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0) {
		fprintf(stderr, "Could not initialize SDL: %s\n", SDL_GetError());
		return 1;
	}

	display = SDL_SetVideoMode(640, 500, 0, SDL_SWSURFACE);
	if (display == NULL) {
		fprintf(stderr, "Could not open display: %s\n", SDL_GetError());
		goto fail;
	}

	GP_SDL_ContextFromSurface(&context, display);

	white_pixel     = GP_ColorToContextPixel(GP_COL_WHITE, &context);
	gray_pixel      = GP_ColorToContextPixel(GP_COL_GRAY_LIGHT, &context);
	dark_gray_pixel = GP_ColorToContextPixel(GP_COL_GRAY_DARK, &context);
	black_pixel     = GP_ColorToContextPixel(GP_COL_BLACK, &context);
	red_pixel       = GP_ColorToContextPixel(GP_COL_RED, &context);
	blue_pixel      = GP_ColorToContextPixel(GP_COL_BLUE, &context);

	redraw_screen();
	SDL_Flip(display);

	event_loop();

	SDL_Quit();
	return 0;

fail:
	SDL_Quit();
	return 1;
}

