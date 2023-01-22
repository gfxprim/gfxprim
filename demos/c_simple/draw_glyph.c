// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2023 Cyril Hrubis <metan@ucw.cz>
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <gfxprim.h>

static gp_backend *backend;

static gp_pixel black_pixel, red_pixel, blue_pixel, green_pixel, gray_pixel;

static int tracking = 0;

static int mul = 1;
static int space = 0;

static const char *font_face;
static gp_font_face *font;
static gp_text_style style = GP_DEFAULT_TEXT_STYLE;

#define TEXT "GFXprim \u00abUnicode\u00bb glyphs."

void redraw_screen(void)
{
	gp_pixmap *pix = backend->pixmap;

	gp_fill(pix, black_pixel);

	style.pixel_xmul = mul;
	style.pixel_ymul = mul;
	style.pixel_xspace = space;
	style.pixel_yspace = space;
	style.char_xspace = tracking;

	uint32_t glyph;
	const char *str = TEXT;
	uint8_t flags = 0;
	gp_coord x = 16;
	int i = 0;
	gp_pixel colors[] = {
		red_pixel,
		green_pixel,
		blue_pixel
	};

	while ((glyph = gp_utf8_next(&str))) {
		x += gp_glyph_draw(pix, &style, x, 16, flags,
		                   colors[i], black_pixel, glyph);

		i = (i+1)%GP_ARRAY_SIZE(colors);

		flags = GP_TEXT_BEARING;
	}

	gp_text(pix, &style, 16, 32 + gp_text_height(&style),
		GP_VALIGN_BELOW|GP_ALIGN_RIGHT,
		gray_pixel, black_pixel, "Plain old text");
}

static void next_font(int dir)
{
	static gp_fonts_iter iter;
	const gp_font_family *family;
	int wrap = font ? style.font == font : 1;

	style.font = gp_fonts_iter_font(&iter, wrap, dir);

	if (!style.font) {
		style.font = font;
		printf("Font: '%s'\n", font_face);
		return;
	}

	family = gp_fonts_iter_family(&iter, 0, GP_FONTS_ITER_NOP);

	printf("Font family: '%s' Font style: '%s'\n",
	       family->family_name, gp_font_style_name(style.font->style));
}

void event_loop(void)
{
	for (;;) {
		gp_event *ev = gp_backend_wait_event(backend);

		switch (ev->type) {
		case GP_EV_KEY:
			if (ev->code != GP_EV_KEY_DOWN)
				continue;

			switch (ev->key.key) {
			case GP_KEY_SPACE:
				next_font(GP_FONTS_ITER_NEXT);
				redraw_screen();
				gp_backend_flip(backend);
			break;
			case GP_KEY_BACKSPACE:
				next_font(GP_FONTS_ITER_PREV);
				redraw_screen();
				gp_backend_flip(backend);
			break;
			case GP_KEY_RIGHT:
				tracking++;
				redraw_screen();
				gp_backend_flip(backend);
			break;
			case GP_KEY_LEFT:
				tracking--;
				redraw_screen();
				gp_backend_flip(backend);
			break;
			case GP_KEY_DOT:
				space++;
				redraw_screen();
				gp_backend_flip(backend);
			break;
			case GP_KEY_COMMA:
				space--;
				redraw_screen();
				gp_backend_flip(backend);
			break;
			case GP_KEY_RIGHT_BRACE:
				mul++;
				redraw_screen();
				gp_backend_flip(backend);
			break;
			case GP_KEY_LEFT_BRACE:
				if (mul > 0)
					mul--;
				redraw_screen();
				gp_backend_flip(backend);
			break;
			case GP_KEY_R:
				gp_pixmap_rotate_cw(backend->pixmap);
				redraw_screen();
				gp_backend_flip(backend);
			break;
			case GP_KEY_ESC:
				gp_backend_exit(backend);
				exit(0);
			break;
			}
		break;
		case GP_EV_SYS:
			switch(ev->code) {
			case GP_EV_SYS_QUIT:
				gp_backend_exit(backend);
				exit(0);
			break;
			case GP_EV_SYS_RESIZE:
				gp_backend_resize_ack(backend);
				redraw_screen();
				gp_backend_flip(backend);
			break;
			}
		break;
		}
	}
}

int main(int argc, char *argv[])
{
	const char *backend_opts = "X11";
	int opt;

	while ((opt = getopt(argc, argv, "b:f:h")) != -1) {
		switch (opt) {
		case 'b':
			backend_opts = optarg;
		break;
		case 'f':
			font_face = optarg;
		break;
		case 'h':
			printf("Usage: %s [-b backend] [-f font_face]\n\n", argv[0]);
			gp_backend_init(NULL, NULL);
			return 0;
		default:
			fprintf(stderr, "Invalid paramter '%c'\n", opt);
			return 1;
		}
	}

	if (font_face)
		font = gp_font_face_load(font_face, 0, 16);

	backend = gp_backend_init(backend_opts, "File View");
	if (!backend) {
		fprintf(stderr, "Failed to initalize backend '%s'\n",
		        backend_opts);
		return 1;
	}

	gp_pixmap *win = backend->pixmap;

	black_pixel = gp_rgb_to_pixmap_pixel(0x00, 0x00, 0x00, win);
	red_pixel   = gp_rgb_to_pixmap_pixel(0xff, 0x00, 0x00, win);
	blue_pixel  = gp_rgb_to_pixmap_pixel(0x00, 0x00, 0xff, win);
	green_pixel = gp_rgb_to_pixmap_pixel(0x00, 0xff, 0x00, win);
	gray_pixel  = gp_rgb_to_pixmap_pixel(0xbb, 0xbb, 0xbb, win);

	next_font(GP_FONTS_ITER_FIRST);

	redraw_screen();
	gp_backend_flip(backend);

	event_loop();

	return 0;
}
