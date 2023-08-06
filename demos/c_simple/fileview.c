// SPDX-License-Identifier: GPL-2.1-or-later
/*
 * Copyright (C) 2009-2010 Jiri "BlueBear" Dluhos
 *                         <jiri.bluebear.dluhos@gmail.com>
 *
 * Copyright (C) 2009-2013 Cyril Hrubis <metan@ucw.cz>
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <gfxprim.h>

static gp_backend *backend;

static gp_pixel white_pixel, gray_pixel, dark_gray_pixel, black_pixel,
		red_pixel, blue_pixel;

static int tracking = 0;

static int mul = 1;
static int space = 0;

static const char *font_face;
static gp_font_face *font;
static gp_text_style style = GP_DEFAULT_TEXT_STYLE;

struct file_line {
	char *text;
	struct file_line *next;
	struct file_line *prev;
};

struct file_line *first_line = NULL;
struct file_line *last_line = NULL;

void redraw_screen(void)
{
	gp_pixmap *win = backend->pixmap;

	gp_fill(win, gray_pixel);

	style.pixel_xmul = mul;
	style.pixel_ymul = mul;
	style.pixel_xspace = space;
	style.pixel_yspace = space;
	style.char_xspace = tracking;

	/* Text alignment (we are always drawing to the right
	 * and below the starting point).
	 */
	int align = GP_ALIGN_RIGHT|GP_VALIGN_BELOW;

	struct file_line *line = first_line;
	unsigned int i;
	for (i = 0; i < win->h/gp_text_height(&style); i++) {
		if (line == NULL)
			break;
		gp_text(win, &style, 16, 16 + (1.0 * gp_text_height(&style))*i,
		        align, black_pixel, gray_pixel, line->text);
		line = line->next;
	}
}

static void warp_up(int lines)
{
	while (lines-- > 0) {
		if (first_line->prev != NULL)
			first_line = first_line->prev;
	}

	redraw_screen();
	gp_backend_flip(backend);
}

static void warp_down(int lines)
{
	while (lines-- > 0) {
		if (first_line->next != NULL)
			first_line = first_line->next;
	}

	redraw_screen();
	gp_backend_flip(backend);
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
			case GP_KEY_UP:
				warp_up(1);
			break;
			case GP_KEY_DOWN:
				warp_down(1);
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
			case GP_KEY_PAGE_UP:
				warp_up(30);
			break;
			case GP_KEY_PAGE_DOWN:
				warp_down(30);
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

		struct file_line *line = malloc(sizeof(*line));
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
	const char *backend_opts = NULL;
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
			printf("Usage: %s [-b backend] [-f font_face] filename\n\n", argv[0]);
			gp_backend_init_help();
			return 0;
		default:
			fprintf(stderr, "Invalid paramter '%c'\n", opt);
			return 1;
		}
	}

	if (font_face)
		font = gp_font_face_load(font_face, 0, 16);

	if (optind >= argc)
		fprintf(stderr, "Expected filename\n");

	if (!read_file_head(argv[optind]))
		return 1;

	backend = gp_backend_init(backend_opts, 0, 0, "File View");

	if (backend == NULL) {
		fprintf(stderr, "Failed to initalize backend '%s'\n",
		        backend_opts);
		return 1;
	}

	gp_pixmap *win = backend->pixmap;

	white_pixel     = gp_rgb_to_pixmap_pixel(0xff, 0xff, 0xff, win);
	gray_pixel      = gp_rgb_to_pixmap_pixel(0xbe, 0xbe, 0xbe, win);
	dark_gray_pixel = gp_rgb_to_pixmap_pixel(0x7f, 0x7f, 0x7f, win);
	black_pixel     = gp_rgb_to_pixmap_pixel(0x00, 0x00, 0x00, win);
	red_pixel       = gp_rgb_to_pixmap_pixel(0xff, 0x00, 0x00, win);
	blue_pixel      = gp_rgb_to_pixmap_pixel(0x00, 0x00, 0xff, win);

	next_font(GP_FONTS_ITER_FIRST);

	redraw_screen();
	gp_backend_flip(backend);

	event_loop();

	return 0;
}
