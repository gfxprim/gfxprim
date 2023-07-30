// SPDX-License-Identifier: GPL-2.1-or-later
/*
 * Copyright (C) 2009-2010 Jiri "BlueBear" Dluhos
 *                         <jiri.bluebear.dluhos@gmail.com>
 *
 * Copyright (C) 2009-2014 Cyril Hrubis <metan@ucw.cz>
 */

#include <stdio.h>
#include <gfxprim.h>

static gp_pixel black_pixel, red_pixel, yellow_pixel, green_pixel, blue_pixel,
		darkgray_pixel, white_pixel;

static int X = 640;
static int Y = 480;

static gp_font_face *font;
static gp_text_style style = GP_DEFAULT_TEXT_STYLE;
static const char *font_face;

static gp_backend *win;

void redraw_screen(void)
{
	gp_fill(win->pixmap, black_pixel);

	/* draw axes intersecting in the middle, where text should be shown */
	gp_hline(win->pixmap, 0, X, Y/2, darkgray_pixel);
	gp_vline(win->pixmap, X/2, 0, Y, darkgray_pixel);

	gp_text(win->pixmap, &style, X/2, Y/2, GP_ALIGN_LEFT|GP_VALIGN_BELOW,
	        yellow_pixel, black_pixel, "bottom left");
	gp_text(win->pixmap, &style, X/2, Y/2, GP_ALIGN_RIGHT|GP_VALIGN_BELOW,
	        red_pixel, black_pixel, "bottom right");
	gp_text(win->pixmap, &style, X/2, Y/2, GP_ALIGN_RIGHT|GP_VALIGN_ABOVE,
	        blue_pixel, black_pixel, "top right");
	gp_text(win->pixmap, &style, X/2, Y/2, GP_ALIGN_LEFT|GP_VALIGN_ABOVE,
	        green_pixel, black_pixel, "top left");

	gp_hline(win->pixmap, 0, X, Y/3, darkgray_pixel);
	gp_text(win->pixmap, &style, X/2, Y/3, GP_ALIGN_CENTER|GP_VALIGN_BASELINE,
	        white_pixel, black_pixel, "x center y baseline");
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

static void event_loop(void)
{
	for (;;) {
		gp_event *ev = gp_backend_wait_event(win);

		switch (ev->type) {
		case GP_EV_KEY:
			if (ev->code != GP_EV_KEY_DOWN)
				continue;

			switch (ev->key.key) {
			case GP_KEY_X:
				win->pixmap->x_swap = !win->pixmap->x_swap;
			break;
			case GP_KEY_Y:
				win->pixmap->y_swap = !win->pixmap->y_swap;
			break;
			case GP_KEY_R:
				win->pixmap->axes_swap = !win->pixmap->axes_swap;
				GP_SWAP(X, Y);
			break;
			case GP_KEY_SPACE:
				next_font(GP_FONTS_ITER_NEXT);
			break;
			case GP_KEY_BACKSPACE:
				next_font(GP_FONTS_ITER_PREV);
			break;
			case GP_KEY_UP:
				style.pixel_xspace++;
				style.pixel_yspace++;
			break;
			case GP_KEY_DOWN:
				style.pixel_xspace--;
				style.pixel_yspace--;
			break;
			case GP_KEY_RIGHT:
				style.pixel_xmul++;
				style.pixel_ymul++;
			break;
			case GP_KEY_LEFT:
				style.pixel_xmul--;
				style.pixel_ymul--;
			break;
			case GP_KEY_ESC:
				gp_backend_exit(win);
				exit(0);
			break;
			}
		break;
		case GP_EV_SYS:
			switch(ev->code) {
			case GP_EV_SYS_QUIT:
				gp_backend_exit(win);
				exit(0);
			break;
			case GP_EV_SYS_RESIZE:
				gp_backend_resize_ack(win);
				X = win->pixmap->w;
				Y = win->pixmap->h;
			break;
			}
		break;
		}

		redraw_screen();
		gp_backend_flip(win);
	}
}

void print_instructions(void)
{
	printf("Use the following keys to control the test:\n");
	printf("    Space ........ toggle font\n");
	printf("    X ............ mirror X\n");
	printf("    Y ............ mirror Y\n");
	printf("    R ............ reverse X and Y\n");
	printf("    UP/DOWN ...... increase/decrease X and Y space\n");
	printf("    RIGHT/LEFT ... increase/decrease X and Y mul\n");
}

int main(int argc, char *argv[])
{
	const char *backend_opts = NULL;
	int opt;

	print_instructions();

	while ((opt = getopt(argc, argv, "b:f:h")) != -1) {
		switch (opt) {
		case 'b':
			backend_opts = optarg;
		break;
		case 'f':
			font_face = optarg;
		break;
		case 'h':
			printf("Usage: %s [-b backend] [-f font_face] filename\n\n", argv[0]);
			gp_backend_init(NULL, NULL);
			return 0;
		default:
			fprintf(stderr, "Invalid paramter '%c'\n", opt);
			return 1;
		}
	}

	if (font_face)
		font = gp_font_face_load(font_face, 0, 20);

	win = gp_backend_init(backend_opts, "Font Align Test");

	if (win == NULL) {
		fprintf(stderr, "Failed to initalize backend '%s'\n",
		        backend_opts);
		return 1;
	}

	black_pixel    = gp_rgb_to_pixmap_pixel(0x00, 0x00, 0x00, win->pixmap);
	red_pixel      = gp_rgb_to_pixmap_pixel(0xff, 0x00, 0x00, win->pixmap);
	blue_pixel     = gp_rgb_to_pixmap_pixel(0x00, 0x00, 0xff, win->pixmap);
	green_pixel    = gp_rgb_to_pixmap_pixel(0x00, 0xff, 0x00, win->pixmap);
	yellow_pixel   = gp_rgb_to_pixmap_pixel(0xff, 0xff, 0x00, win->pixmap);
	white_pixel   = gp_rgb_to_pixmap_pixel(0xff, 0xff, 0xff, win->pixmap);
	darkgray_pixel = gp_rgb_to_pixmap_pixel(0x7f, 0x7f, 0x7f, win->pixmap);

	next_font(GP_FONTS_ITER_FIRST);

	redraw_screen();
	gp_backend_flip(win);
	event_loop();

	return 0;
}

