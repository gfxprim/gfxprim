// SPDX-License-Identifier: GPL-2.1-or-later
/*
 * Copyright (C) 2009-2010 Jiri "BlueBear" Dluhos
 *                         <jiri.bluebear.dluhos@gmail.com>
 *
 * Copyright (C) 2009-2013 Cyril Hrubis <metan@ucw.cz>
 */

#include <stdio.h>
#include <gfxprim.h>

static gp_pixel black;
static gp_pixel white;

static gp_backend *win;

static gp_pixmap *bitmap, *bitmap_raw, *bitmap_conv;
static int bitmap_x, bitmap_y, bitmap_vx = -3, bitmap_vy = -3;
static int pause_flag = 0;

static char text_buf[255];

void redraw_screen(void)
{
	bitmap_x += bitmap_vx;
	bitmap_y += bitmap_vy;

	if (bitmap_x + gp_pixmap_w(bitmap) > win->pixmap->w) {
		bitmap_vx = -bitmap_vx;
		bitmap_x += bitmap_vx;
	}

	if (bitmap_x < 0) {
		bitmap_vx = -bitmap_vx;
		bitmap_x += bitmap_vx;
	}

	if (bitmap_y + gp_pixmap_h(bitmap) > win->pixmap->h) {
		bitmap_vy = -bitmap_vy;
		bitmap_y += bitmap_vy;
	}

	if (bitmap_y < 0) {
		bitmap_vy = -bitmap_vy;
		bitmap_y += bitmap_vy;
	}

	gp_fill_rect_xywh(win->pixmap, 20, 20, 300, 50, black);

	gp_text(win->pixmap, NULL, 20, 20, GP_ALIGN_RIGHT|GP_VALIGN_BOTTOM,
	        white, black, text_buf);

	gp_print(win->pixmap, NULL, 250, 20, GP_ALIGN_RIGHT|GP_VALIGN_BOTTOM,
	         white, black, "%c|%c|%c", bitmap->x_swap ? 'x' : ' ',
		 bitmap->y_swap ? 'y' : ' ', bitmap->axes_swap ? 'a' : ' ');

	gp_blit(bitmap, 0, 0, gp_pixmap_w(bitmap), gp_pixmap_h(bitmap),
		win->pixmap, bitmap_x, bitmap_y);

	gp_backend_update_rect_xywh(win, bitmap_x, bitmap_y,
	                     gp_pixmap_w(bitmap), gp_pixmap_h(bitmap));
	gp_backend_update_rect_xywh(win, 20, 20, 400, 50);
}

static void change_bitmap(void)
{
	if (bitmap == bitmap_raw)
		bitmap = bitmap_conv;
	else
		bitmap = bitmap_raw;

	snprintf(text_buf, sizeof(text_buf), "'%s' -> '%s'",
	         gp_pixel_type_name(bitmap->pixel_type),
		 gp_pixel_type_name(win->pixmap->pixel_type));
}

void event_loop(void)
{
	gp_event ev;

	while (gp_backend_get_event(win, &ev)) {
		gp_event_dump(&ev);

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
				gp_fill(win->pixmap, black);
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

	bitmap_raw = gp_load_image(sprite, NULL);

	if (!bitmap_raw) {
		fprintf(stderr, "Failed to load '%s'\n", sprite);
		return 1;
	}

	win = gp_backend_init(backend_opts, "Blit Test");

	if (win == NULL) {
		fprintf(stderr, "Failed to initalize backend '%s'\n",
		        backend_opts);
		return 1;
	}

	bitmap_conv = gp_pixmap_convert_alloc(bitmap_raw,
					      win->pixmap->pixel_type);
	change_bitmap();

	black = gp_rgb_to_pixmap_pixel(0x00, 0x00, 0x00, win->pixmap);
	white = gp_rgb_to_pixmap_pixel(0xff, 0xff, 0xff, win->pixmap);

	gp_fill(win->pixmap, black);
	gp_backend_flip(win);

	for (;;) {
		gp_backend_poll(win);
		event_loop();

		usleep(8000);

		if (pause_flag)
			continue;

		redraw_screen();
	}
}

