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
#include <GP.h>

static GP_Pixel black_pixel, red_pixel, yellow_pixel, green_pixel, blue_pixel,
		darkgray_pixel, white_pixel;

static int font_flag = 0;

static int X = 640;
static int Y = 480;

static GP_FontFace *font = NULL;
static GP_TextStyle style = GP_DEFAULT_TEXT_STYLE;

static GP_Backend *win;

void redraw_screen(void)
{
	GP_Fill(win->context, black_pixel);

	/* draw axes intersecting in the middle, where text should be shown */
	GP_HLine(win->context, 0, X, Y/2, darkgray_pixel);
	GP_VLine(win->context, X/2, 0, Y, darkgray_pixel);

	switch (font_flag) {
	case 0:
		style.font = &GP_DefaultProportionalFont;
	break;
	case 1:
		style.font = &GP_DefaultConsoleFont;
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

	GP_Text(win->context, &style, X/2, Y/2, GP_ALIGN_LEFT|GP_VALIGN_BELOW,
	        yellow_pixel, black_pixel, "bottom left");
	GP_Text(win->context, &style, X/2, Y/2, GP_ALIGN_RIGHT|GP_VALIGN_BELOW,
	        red_pixel, black_pixel, "bottom right");
	GP_Text(win->context, &style, X/2, Y/2, GP_ALIGN_RIGHT|GP_VALIGN_ABOVE,
	        blue_pixel, black_pixel, "top right");
	GP_Text(win->context, &style, X/2, Y/2, GP_ALIGN_LEFT|GP_VALIGN_ABOVE,
	        green_pixel, black_pixel, "top left");

	GP_HLine(win->context, 0, X, Y/3, darkgray_pixel);
	GP_Text(win->context, &style, X/2, Y/3, GP_ALIGN_CENTER|GP_VALIGN_BASELINE,
	        white_pixel, black_pixel, "x center y baseline");
}

static void event_loop(void)
{
	GP_Event ev;

	for (;;) {
		GP_BackendWaitEvent(win, &ev);

		switch (ev.type) {
		case GP_EV_KEY:
			if (ev.code != GP_EV_KEY_DOWN)
				continue;

			switch (ev.val.key.key) {
			case GP_KEY_X:
				win->context->x_swap = !win->context->x_swap;
			break;
			case GP_KEY_Y:
				win->context->y_swap = !win->context->y_swap;
			break;
			case GP_KEY_R:
				win->context->axes_swap = !win->context->axes_swap;
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
				GP_BackendExit(win);
				exit(0);
			break;
			}
		break;
		case GP_EV_SYS:
			switch(ev.code) {
			case GP_EV_SYS_QUIT:
				GP_BackendExit(win);
				exit(0);
			break;
			case GP_EV_SYS_RESIZE:
				GP_BackendResizeAck(win);
				X = win->context->w;
				Y = win->context->h;
			break;
			}
		break;
		}

		redraw_screen();
		GP_BackendFlip(win);
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
		font = GP_FontFaceLoad(argv[1], 0, 20);

	print_instructions();

	win = GP_BackendInit(backend_opts, "Font Align Test");

	if (win == NULL) {
		fprintf(stderr, "Failed to initalize backend '%s'\n",
		        backend_opts);
		return 1;
	}

	black_pixel    = GP_RGBToContextPixel(0x00, 0x00, 0x00, win->context);
	red_pixel      = GP_RGBToContextPixel(0xff, 0x00, 0x00, win->context);
	blue_pixel     = GP_RGBToContextPixel(0x00, 0x00, 0xff, win->context);
	green_pixel    = GP_RGBToContextPixel(0x00, 0xff, 0x00, win->context);
	yellow_pixel   = GP_RGBToContextPixel(0xff, 0xff, 0x00, win->context);
	white_pixel   = GP_RGBToContextPixel(0xff, 0xff, 0xff, win->context);
	darkgray_pixel = GP_RGBToContextPixel(0x7f, 0x7f, 0x7f, win->context);

	redraw_screen();
	GP_BackendFlip(win);
	event_loop();

	return 0;
}

