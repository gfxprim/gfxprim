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
 * Copyright (C) 2009-2014 Cyril Hrubis <metan@ucw.cz>                       *
 *                                                                           *
 *****************************************************************************/

#include <stdio.h>
#include <gfxprim.h>

static gp_pixel black_pixel, red_pixel, yellow_pixel, green_pixel, blue_pixel,
		darkgray_pixel, white_pixel;

static int font_flag = 0;

static int X = 640;
static int Y = 480;

static gp_font_face *font = NULL;
static gp_text_style style = GP_DEFAULT_TEXT_STYLE;

static gp_backend *win;

void redraw_screen(void)
{
	gp_fill(win->pixmap, black_pixel);

	/* draw axes intersecting in the middle, where text should be shown */
	gp_hline(win->pixmap, 0, X, Y/2, darkgray_pixel);
	gp_vline(win->pixmap, X/2, 0, Y, darkgray_pixel);

	switch (font_flag) {
	case 0:
		style.font = gp_font_gfxprim;
	break;
	case 1:
		style.font = gp_font_gfxprim_mono;
	break;
	case 2:
		style.font = gp_font_tiny_mono;
	break;
	case 3:
		style.font = gp_font_tiny;
	break;
	case 4:
		style.font = gp_font_c64;
	break;
	case 5:
		style.font = font;
	break;
	}

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

static void event_loop(void)
{
	gp_event ev;

	for (;;) {
		gp_backend_wait_event(win, &ev);

		switch (ev.type) {
		case GP_EV_KEY:
			if (ev.code != GP_EV_KEY_DOWN)
				continue;

			switch (ev.val.key.key) {
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
				font_flag++;

				if (font) {
					if (font_flag > 5)
						font_flag = 0;
				} else {
					if (font_flag > 4)
						font_flag = 0;
				}
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
			switch(ev.code) {
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
	const char *backend_opts = "X11";

	if (argc > 1)
		font = gp_font_face_load(argv[1], 0, 20);

	print_instructions();

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

	redraw_screen();
	gp_backend_flip(win);
	event_loop();

	return 0;
}

