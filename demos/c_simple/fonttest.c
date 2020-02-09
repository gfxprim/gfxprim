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

#include <gfxprim.h>

static gp_backend *win;

static const char *font_path = NULL;
static unsigned int font_h = 16;

static gp_pixel white_pixel, gray_pixel, dark_gray_pixel, black_pixel,
		red_pixel, blue_pixel;

static const char *test_strings[] = {
	" !\"#$%&\047()*+,-./0123456789:;<=>?@",
	"ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`",
	"abcdefghijklmnopqrstuvwxyz{|}~",
	"Lorem ipsum dolor sit amet, consectetur adipisicing elit, sed do eiusmod tempor..."
};

static int font_flag = 0;
static int tracking = 0;
static gp_font_face *font = NULL;

static const char *glyph_bitmap_format_name(const gp_font_bitmap_format format)
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

static void print_character_metadata(const gp_font_face *font, int c)
{
	const gp_glyph *glyph = gp_get_glyph(font, c);
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

static void print_font_properties(const gp_font_face *font)
{
	fprintf(stderr, "Font '%s %s' properties:\n",
	                gp_font_family(font), gp_font_style(font));
	fprintf(stderr, "    Height: ascend: %d, descend: %d\n",
			gp_font_ascend(font), gp_font_descend(font));
	fprintf(stderr, "    Max advance_x: %u\n",
	                gp_font_max_advance_x(font));
	fprintf(stderr, "    Glyph bitmap format: %s\n",
	                glyph_bitmap_format_name(font->glyph_bitmap_format));
	fprintf(stderr, "    Bounding box width: %d, heigth: %d\n",
	                gp_font_max_width(font), gp_font_height(font));

	print_character_metadata(font, 'a');
	print_character_metadata(font, 'm');
	print_character_metadata(font, '0');
}

#define SPACING 120

void redraw_screen(void)
{
	gp_fill(win->pixmap, black_pixel);

	gp_text_style style = GP_DEFAULT_TEXT_STYLE;

	switch (font_flag) {
	case 0:
		style.font = gp_font_gfxprim;
	break;
	case 1:
		style.font = gp_font_gfxprim_mono;
	break;
	case 2:
		style.font = gp_font_tiny;
	break;
	case 3:
		style.font = gp_font_tiny_mono;
	break;
	case 4:
		style.font = gp_font_c64;
	break;
	case 5:
		style.font = gp_font_haxor_narrow_15;
	break;
	case 6:
		style.font = gp_font_haxor_narrow_16;
	break;
	case 7:
		style.font = gp_font_haxor_narrow_17;
	break;
	case 8:
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

		gp_fill_rect_xywh(win->pixmap,
			16, SPACING*i + 16,
			gp_text_width(&style, test_string),
			gp_font_height(style.font),
			dark_gray_pixel);

		gp_rect_xywh(win->pixmap,
			15, SPACING*i + 15,
			gp_text_max_width(&style, strlen(test_string)) + 1,
			gp_font_height(style.font) + 1,
			blue_pixel);

		gp_hline_xyw(win->pixmap, 15,
		             SPACING*i + 15 + gp_font_ascend(style.font),
		             gp_text_max_width(&style, strlen(test_string)) + 1,
		             blue_pixel);

		gp_text(win->pixmap, &style, 16, SPACING*i + 16, align,
		        white_pixel, dark_gray_pixel, test_string);


		style.pixel_xmul = 2;
		style.pixel_ymul = 2;
		style.pixel_yspace = 1;

		gp_text(win->pixmap, &style, 34, SPACING * i + 44, align,
		        white_pixel, black_pixel, test_string);

		gp_rect_xywh(win->pixmap, 33, SPACING * i + 43,
		             gp_text_width(&style, test_string) + 1,
			     gp_text_height(&style) + 1, dark_gray_pixel);

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

		gp_text(win->pixmap, &style, 64, SPACING*i + 88, align,
		        dark_gray_pixel, black_pixel, test_string);
	}
}

void event_loop(void)
{
	gp_event ev;

	for (;;) {
		gp_backend_wait_event(win, &ev);

		switch (ev.type) {
		case GP_EV_KEY:
			if (ev.code != GP_EV_KEY_DOWN)
				continue;

			switch (ev.val.key.key) {
			case GP_KEY_SPACE:
				if (font)
					font_flag = (font_flag + 1) % 9;
				else
					font_flag = (font_flag + 1) % 8;

				redraw_screen();
				gp_backend_flip(win);
			break;
			case GP_KEY_UP:
				tracking++;
				redraw_screen();
				gp_backend_flip(win);
			break;
			case GP_KEY_DOWN:
				tracking--;
				redraw_screen();
				gp_backend_flip(win);
			break;
			case GP_KEY_B:
				font_h++;
				if (font_path) {
					gp_font_face_free(font);
					font = gp_font_face_load(font_path, 0, font_h);
					redraw_screen();
					gp_backend_flip(win);
				}
			break;
			case GP_KEY_S:
				font_h--;
				if (font_path) {
					gp_font_face_free(font);
					font = gp_font_face_load(font_path, 0, font_h);
					redraw_screen();
					gp_backend_flip(win);
				}
			break;
			case GP_KEY_ESC:
				gp_backend_exit(win);
				exit(0);
			break;
			}
		break;
		case GP_EV_SYS:
			switch(ev.code) {
			case GP_EV_SYS_RESIZE:
				gp_backend_resize_ack(win);
				redraw_screen();
				gp_backend_flip(win);
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
		font = gp_font_face_load(argv[1], 0, font_h);
	}

	win = gp_backend_init(backend_opts, "Font Test");

	if (win == NULL) {
		fprintf(stderr, "Failed to initalize backend '%s'\n",
		        backend_opts);
		return 1;
	}

	white_pixel     = gp_rgb_to_pixmap_pixel(0xff, 0xff, 0xff, win->pixmap);
	gray_pixel      = gp_rgb_to_pixmap_pixel(0xbe, 0xbe, 0xbe, win->pixmap);
	dark_gray_pixel = gp_rgb_to_pixmap_pixel(0x7f, 0x7f, 0x7f, win->pixmap);
	black_pixel     = gp_rgb_to_pixmap_pixel(0x00, 0x00, 0x00, win->pixmap);
	red_pixel       = gp_rgb_to_pixmap_pixel(0xff, 0x00, 0x00, win->pixmap);
	blue_pixel      = gp_rgb_to_pixmap_pixel(0x00, 0x00, 0xff, win->pixmap);

	redraw_screen();
	gp_backend_flip(win);

	event_loop();

	return 0;
}
