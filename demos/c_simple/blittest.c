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

#include <GP.h>

static GP_Pixel black;
static GP_Pixel white;

static GP_Backend *win;

static GP_Context *bitmap, *bitmap_raw, *bitmap_conv;
static int bitmap_x, bitmap_y, bitmap_vx = -3, bitmap_vy = -3;
static int pause_flag = 0;

static char text_buf[255];

void redraw_screen(void)
{
	bitmap_x += bitmap_vx;
	bitmap_y += bitmap_vy;

	if (bitmap_x + GP_ContextW(bitmap) > win->context->w) {
		bitmap_vx = -bitmap_vx;
		bitmap_x += bitmap_vx;
	}

	if (bitmap_x < 0) {
		bitmap_vx = -bitmap_vx;
		bitmap_x += bitmap_vx;
	}

	if (bitmap_y + GP_ContextH(bitmap) > win->context->h) {
		bitmap_vy = -bitmap_vy;
		bitmap_y += bitmap_vy;
	}
	
	if (bitmap_y < 0) {
		bitmap_vy = -bitmap_vy;
		bitmap_y += bitmap_vy;
	}

	GP_FillRectXYWH(win->context, 20, 20, 300, 50, black);
	
	GP_Text(win->context, NULL, 20, 20, GP_ALIGN_RIGHT|GP_VALIGN_BOTTOM,
	        white, black, text_buf);

	GP_Print(win->context, NULL, 250, 20, GP_ALIGN_RIGHT|GP_VALIGN_BOTTOM,
	         white, black, "%c|%c|%c", bitmap->x_swap ? 'x' : ' ',
		 bitmap->y_swap ? 'y' : ' ', bitmap->axes_swap ? 'a' : ' ');
	
	GP_Blit(bitmap, 0, 0, GP_ContextW(bitmap), GP_ContextH(bitmap),
	        win->context, bitmap_x, bitmap_y);

	GP_BackendUpdateRectXYWH(win, bitmap_x, bitmap_y,
	                     GP_ContextW(bitmap), GP_ContextH(bitmap));
	GP_BackendUpdateRectXYWH(win, 20, 20, 400, 50);
}

static void change_bitmap(void)
{
	if (bitmap == bitmap_raw)
		bitmap = bitmap_conv;
	else
		bitmap = bitmap_raw;
	
	snprintf(text_buf, sizeof(text_buf), "'%s' -> '%s'",
	         GP_PixelTypeName(bitmap->pixel_type),
		 GP_PixelTypeName(win->context->pixel_type));
}

void event_loop(void)
{
	GP_Event ev;

	while (GP_BackendGetEvent(win, &ev)) {
		GP_EventDump(&ev);
		
		switch (ev.type) {
		case GP_EV_KEY:
			if (ev.code != GP_EV_KEY_DOWN)
				continue;
			
			switch (ev.val.key.key) {
			case GP_KEY_X:
				bitmap->x_swap = !bitmap->x_swap;
			break;
			case GP_KEY_Y:
				bitmap->y_swap = !bitmap->y_swap;
			break;
			case GP_KEY_R:
				bitmap->axes_swap = !bitmap->axes_swap;
			break;
			case GP_KEY_P:
				pause_flag = !pause_flag;
			break;
			case GP_KEY_SPACE:
				change_bitmap();
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
			}
		break;
		}
	}
}

void print_instructions(void)
{
	printf("Use the following keys to control the test:\n");
	printf("    Esc ............. exit\n");
	printf("    Space ........... converts bitmap to screen pixel format\n");
	printf("    R ............... swap sprite axes\n");
	printf("    X ............... mirror sprite X\n");
	printf("    Y ............... mirror sprite Y\n");
	printf("    P ............... pause\n");
}

int main(void)
{
	const char *sprite = "ball.ppm";
	const char *backend_opts = "X11";

	print_instructions();
	
	bitmap_raw = GP_LoadImage(sprite, NULL);

	if (!bitmap_raw) {
		fprintf(stderr, "Failed to load '%s'\n", sprite);
		return 1;
	}
	
	win = GP_BackendInit(backend_opts, "Blit Test", stderr);

	if (win == NULL) {
		fprintf(stderr, "Failed to initalize backend '%s'\n",
		        backend_opts);
		return 1;
	}

	bitmap_conv = GP_ContextConvertAlloc(bitmap_raw,
	                                     win->context->pixel_type);
	change_bitmap();

	black = GP_ColorToContextPixel(GP_COL_BLACK, win->context);
	white = GP_ColorToContextPixel(GP_COL_WHITE, win->context);

	GP_Fill(win->context, black);
	GP_BackendFlip(win);

	for (;;) {
		GP_BackendPoll(win);
		event_loop();
	
		usleep(8000);

		if (pause_flag)
			continue;

		redraw_screen();
	}
}

