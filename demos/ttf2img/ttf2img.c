// SPDX-License-Identifier: GPL-2.1-or-later
/*
 * Copyright (C) 2009-2012 Cyril Hrubis <metan@ucw.cz>
 */

 /*

   Uses gfxprim to render small image with defined string.

  */

#include <gfxprim.h>

static const char help[] = {
	"usage: ttf2img -f font.ttf -i file.png -s string -l size [-d debug_level]\n",
};

static void print_help(int i)
{
	fputs(help, stderr);
	exit(i);
}

int main(int argc, char *argv[])
{
	const char *font_path = NULL;
	const char *img_path = NULL;
	const char *string   = "Foo Bar!";
	int opt, debug_level = 0;
	int img_w = 400, img_h = 100;
	int height = 27;

	while ((opt = getopt(argc, argv, "d:f:i:l:s:")) != -1) {
		switch (opt) {
		case 'f':
			font_path = optarg;
		break;
		case 'i':
			img_path = optarg;
		break;
		case 'd':
			debug_level = atoi(optarg);
		break;
		case 'h':
			print_help(0);
		break;
		case 'l':
			height = atoi(optarg);
		break;
		case 's':
			string = optarg;
		break;
		default:
			fprintf(stderr, "Invalid paramter '%c'\n", opt);
			print_help(1);
		}
	}

	if (font_path == NULL || img_path == NULL)
		print_help(1);

	gp_set_debug_level(debug_level);

	gp_pixmap *pixmap = gp_pixmap_alloc(img_w, img_h, GP_PIXEL_RGB888);

	gp_pixel black_pixel = gp_rgb_to_pixmap_pixel(0x00, 0x00, 0x00, pixmap);
	gp_pixel white_pixel = gp_rgb_to_pixmap_pixel(0xff, 0xff, 0xff, pixmap);

	gp_fill(pixmap, white_pixel);

	gp_text_style style = GP_DEFAULT_TEXT_STYLE;

	style.font = gp_font_face_load(font_path, height, 0);

	gp_text(pixmap, &style, img_w/2, img_h/2, GP_ALIGN_CENTER|GP_VALIGN_CENTER,
	        black_pixel, white_pixel, string);

	gp_save_png(pixmap, img_path, NULL);

	gp_pixmap_free(pixmap);

	return 0;
}
