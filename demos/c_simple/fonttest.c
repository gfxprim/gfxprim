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

static GP_Backend *win;

static const char *font_path = NULL;
static unsigned int font_h = 16;

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
static GP_FontFace *font = NULL;

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
	fprintf(stderr, "Font '%s %s' properties:\n",
	                GP_FontFamily(font), GP_FontStyle(font));
	fprintf(stderr, "    Height: ascend: %d, descend: %d\n",
			GP_FontAscend(font), GP_FontDescend(font));
	fprintf(stderr, "    Max advance_x: %u\n",
	                GP_FontMaxAdvanceX(font));
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
	GP_Fill(win->context, black_pixel);

	GP_TextStyle style = GP_DEFAULT_TEXT_STYLE;

	switch (font_flag) {
	case 0:
		style.font = &GP_DefaultProportionalFont;
	break;
	case 1:
		style.font = &GP_DefaultConsoleFont;
	break;
	case 2:
		style.font = GP_FontTiny;
	break;
	case 3:
		style.font = GP_FontTinyMono;
	break;
	case 4:
		style.font = GP_FontC64;
	break;
	case 5:
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

		GP_FillRectXYWH(win->context,
			16, SPACING*i + 16,
			GP_TextWidth(&style, test_string),
			GP_FontHeight(style.font),
			dark_gray_pixel);

		GP_RectXYWH(win->context,
			15, SPACING*i + 15,
			GP_TextMaxWidth(&style, strlen(test_string)) + 1,
			GP_FontHeight(style.font) + 1,
			blue_pixel);

		GP_Text(win->context, &style, 16, SPACING*i + 16, align,
		        white_pixel, dark_gray_pixel, test_string);

		style.pixel_xmul = 2;
		style.pixel_ymul = 2;
		style.pixel_yspace = 1;

		GP_Text(win->context, &style, 34, SPACING * i + 44, align,
		        white_pixel, black_pixel, test_string);

		GP_RectXYWH(win->context, 33, SPACING * i + 43,
		            GP_TextWidth(&style, test_string) + 1,
			    GP_TextHeight(&style) + 1, dark_gray_pixel);

		style.pixel_xmul = 4;
		style.pixel_ymul = 2;

		style.pixel_xspace = 1;
		style.pixel_yspace = 1;

		if (font_flag == 2 || font_flag == 3) {
			style.pixel_xmul = 2;
			style.pixel_ymul = 5;

			style.pixel_xspace = 2;
			style.pixel_yspace = 2;
		}

		GP_Text(win->context, &style, 64, SPACING*i + 88, align,
		        dark_gray_pixel, black_pixel, test_string);
	}
}

void event_loop(void)
{
	GP_Event ev;

	for (;;) {
		GP_BackendWaitEvent(win, &ev);

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
				GP_BackendFlip(win);
			break;
			case GP_KEY_UP:
				tracking++;
				redraw_screen();
				GP_BackendFlip(win);
			break;
			case GP_KEY_DOWN:
				tracking--;
				redraw_screen();
				GP_BackendFlip(win);
			break;
			case GP_KEY_B:
				font_h++;
				if (font_path) {
					GP_FontFaceFree(font);
					font = GP_FontFaceLoad(font_path, 0, font_h);
					redraw_screen();
					GP_BackendFlip(win);
				}
			break;
			case GP_KEY_S:
				font_h--;
				if (font_path) {
					GP_FontFaceFree(font);
					font = GP_FontFaceLoad(font_path, 0, font_h);
					redraw_screen();
					GP_BackendFlip(win);
				}
			break;
			case GP_KEY_ESC:
				GP_BackendExit(win);
				exit(0);
			break;
			}
		break;
		case GP_EV_SYS:
			switch(ev.code) {
			case GP_EV_SYS_RESIZE:
				GP_BackendResizeAck(win);
				redraw_screen();
				GP_BackendFlip(win);
			break;
			}
		break;
		}
	}
}

void print_instructions(void)
{
	printf("Use the following keys to control the test:\n");
	printf("    Esc ................. exit\n");
	printf("    Space ............... change font\n");
	printf("    up/down ............. increase/decrease tracking\n");
	printf("    b/s ................. change font size (freetype only)\n");
}

int main(int argc, char *argv[])
{
	const char *backend_opts = "X11";

	print_instructions();

	if (argc > 1) {
		font_path = argv[1];
		fprintf(stderr, "\nLoading font '%s'\n", argv[1]);
		font = GP_FontFaceLoad(argv[1], 0, font_h);
	}

	win = GP_BackendInit(backend_opts, "Font Test");

	if (win == NULL) {
		fprintf(stderr, "Failed to initalize backend '%s'\n",
		        backend_opts);
		return 1;
	}

	white_pixel     = GP_RGBToContextPixel(0xff, 0xff, 0xff, win->context);
	gray_pixel      = GP_RGBToContextPixel(0xbe, 0xbe, 0xbe, win->context);
	dark_gray_pixel = GP_RGBToContextPixel(0x7f, 0x7f, 0x7f, win->context);
	black_pixel     = GP_RGBToContextPixel(0x00, 0x00, 0x00, win->context);
	red_pixel       = GP_RGBToContextPixel(0xff, 0x00, 0x00, win->context);
	blue_pixel      = GP_RGBToContextPixel(0x00, 0x00, 0xff, win->context);

	redraw_screen();
	GP_BackendFlip(win);

	event_loop();

	return 0;
}
