// SPDX-License-Identifier: GPL-2.1-or-later
/*
 * Copyright (C) 2023 Cyril Hrubis <metan@ucw.cz>
 */

#include <gfxprim.h>

static gp_backend *win;

static gp_pixel white_pixel, gray_pixel, dark_gray_pixel, black_pixel,
		red_pixel, blue_pixel;

static int pixel_size = 1;
static gp_font_face *font;
static gp_text_style style = GP_DEFAULT_TEXT_STYLE;
static gp_text_style desc_style = GP_DEFAULT_TEXT_STYLE;
static const char *font_path;
static const char *font_family_name;

static const char *font_path;
static unsigned int font_h = 16;
static int reverse;

struct codepage {
	const char *name;
	uint32_t first;
	uint32_t last;
};

static struct codepage codepages[] = {
	{.name = "Basic Latin", .first = 0x20, .last = 0x7e},
	{.name = "Latin-1 Supplement", .first = 0xa1, .last = 0xff},
	{.name = "Latin Extended-A", .first = 0x100, .last = 0x17e},
	{.name = "Greek", .first = 0x384, .last = 0x3ce},
	{.name = "Cyrilic", .first = 0x400, .last = 0x45f},
	{.name = "Punctuation", .first = 0x2018, .last = 0x2037},
	{.name = "Hiragana", .first = 0x3041, .last = 0x3096},
	{.name = "Katakana", .first = 0x30a0, .last = 0x30ff},
};

static unsigned int cur_codepage;

unsigned int spacing = 10;

static void redraw_screen(void)
{
	int align = GP_ALIGN_RIGHT|GP_VALIGN_BELOW;
	struct codepage *codepage = &codepages[cur_codepage];
	unsigned int i;
	gp_pixel bg_col = white_pixel, fg_col = black_pixel;

	if (reverse)
		GP_SWAP(fg_col, bg_col);

	gp_fill(win->pixmap, bg_col);

	style.pixel_xmul = pixel_size;
	style.pixel_ymul = pixel_size;

	if (pixel_size > 1) {
		style.pixel_xspace = 1;
		style.pixel_yspace = 1;
	} else {
		style.pixel_xspace = 0;
		style.pixel_yspace = 0;
	}

	gp_size lw = gp_text_max_width(&style, 1);
	gp_size lh = gp_text_height(&style);

	gp_size dh = gp_text_height(&desc_style);
	gp_size dw = gp_text_max_width(&desc_style, 6);
	gp_size wspacing = GP_MAX((dw+1)/2, spacing);

	gp_coord x = wspacing;
	gp_coord y = spacing;

	gp_print(win->pixmap, &style, x, y, align, fg_col, bg_col,
	         "Family: %s Style: %s", font_family_name, gp_font_style_name(style.font->style));
	y += lh + spacing;
	gp_text(win->pixmap, &style, x, y, align, fg_col, bg_col, codepage->name);
	y += lh + spacing;

	for (i = codepage->first; i <= codepage->last; i++) {
		gp_coord gy = y + dh;

		gp_print(win->pixmap, &desc_style, x + lw/2, y,
		         GP_VALIGN_BELOW | GP_ALIGN_CENTER,
		         fg_col, bg_col, "0x%x", i);

		gp_rect_xywh(win->pixmap, x, gy, lw, lh, dark_gray_pixel);
		gp_glyph_draw(win->pixmap, &style, x, gy, align | GP_TEXT_BEARING, fg_col, bg_col, i);

		x += GP_MAX(lw, dw) + spacing;

		if (x + lw >= gp_pixmap_w(win->pixmap)) {
			x = wspacing;
			y += lh + dh + spacing;
		}
	}

	gp_backend_flip(win);
}

static void next_font(int dir)
{
	static gp_fonts_iter iter;
	const gp_font_family *font_family;
	int wrap = font ? style.font == font : 1;

	style.font = gp_fonts_iter_font(&iter, wrap, dir);

	if (!style.font) {
		style.font = font;
		printf("Font: '%s'\n", font_path);
		font_family_name = font_path;
		return;
	}

	font_family = gp_fonts_iter_family(&iter, 0, GP_FONTS_ITER_NOP);

	font_family_name = font_family->family_name;

	printf("Font family: '%s' Font style: '%s'\n",
	       font_family_name, gp_font_style_name(style.font->style));
}

void event_loop(void)
{
	gp_font_face *new_font;

	for (;;) {
		gp_event *ev = gp_backend_ev_wait(win);

		switch (ev->type) {
		case GP_EV_KEY:
			if (ev->code != GP_EV_KEY_DOWN)
				continue;

			switch (ev->key.key) {
			case GP_KEY_R:
				reverse = !reverse;
				redraw_screen();
			break;
			case GP_KEY_SPACE:
				next_font(GP_FONTS_ITER_NEXT);
				redraw_screen();
			break;
			case GP_KEY_BACKSPACE:
				next_font(GP_FONTS_ITER_PREV);
				redraw_screen();
			break;
			case GP_KEY_UP:
				pixel_size++;
				redraw_screen();
			break;
			case GP_KEY_DOWN:
				pixel_size--;
				redraw_screen();
			break;
			case GP_KEY_LEFT:
				if (cur_codepage)
					cur_codepage--;
				else
					cur_codepage = GP_ARRAY_SIZE(codepages) - 1;

				redraw_screen();
			break;
			case GP_KEY_RIGHT:
				if (cur_codepage < GP_ARRAY_SIZE(codepages) - 1)
					cur_codepage++;
				else
					cur_codepage = 0;

				redraw_screen();
			break;
			case GP_KEY_B:
				font_h++;
				if (font_path) {
					new_font = gp_font_face_load(font_path, 0, font_h);

					if (style.font == font)
						style.font = new_font;

					gp_font_face_free(font);
					font = new_font;
					redraw_screen();
				}
			break;
			case GP_KEY_S:
				font_h--;
				if (font_path) {
					new_font = gp_font_face_load(font_path, 0, font_h);

					if (style.font == font)
						style.font = new_font;

					gp_font_face_free(font);
					font = new_font;
					redraw_screen();
				}
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
				redraw_screen();
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
	printf("    Space/Backspace ..... change font\n");
	printf("    Up/Down ............. increase/decrease pixel size\n");
	printf("    Left/Right .......... increase/decrease code page\n");
	printf("    B/S ................. change font size (freetype only)\n");
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
			font_path = optarg;
		break;
		case 'h':
			printf("\nUsage: %s [-b backend] [-f font_path]\n\n", argv[0]);
			gp_backend_init_help();
			return 0;
		default:
			fprintf(stderr, "Invalid paramter '%c'\n", opt);
			return 1;
		}
	}

	if (font_path) {
		fprintf(stderr, "\nLoading font '%s'\n", font_path);
		font = gp_font_face_load(font_path, 0, font_h);
		if (!font)
			fprintf(stderr, "Failed to load font!\n");
	}

	win = gp_backend_init(backend_opts, 800, 600, "Font Test");

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

	next_font(GP_FONTS_ITER_FIRST);

	redraw_screen();

	event_loop();

	return 0;
}
