// SPDX-License-Identifier: GPL-2.1-or-later
/*
 * Copyright (C) 2009-2013 Cyril Hrubis <metan@ucw.cz>
 */

 /*

   Simple backend example.

  */

#include <stdio.h>
#include <gfxprim.h>

static void redraw(struct gp_pixmap *pixmap)
{
	gp_pixel white_pixel, black_pixel;

	black_pixel = gp_rgb_to_pixmap_pixel(0x00, 0x00, 0x00, pixmap);
	white_pixel = gp_rgb_to_pixmap_pixel(0xff, 0xff, 0xff, pixmap);

	gp_fill(pixmap, black_pixel);
	gp_line(pixmap, 0, 0, pixmap->w - 1, pixmap->h - 1, white_pixel);
	gp_line(pixmap, 0, pixmap->h - 1, pixmap->w - 1, 0, white_pixel);
}

static int ev_loop(struct gp_backend *backend, const char *name)
{
	gp_event *ev;

	if (backend == NULL)
		return 0;

	while ((ev = gp_backend_get_event(backend))) {

		printf("-------------------------- %s\n", name);

		gp_ev_dump(ev);

		switch (ev->type) {
		case GP_EV_KEY:
			switch (ev->val) {
			case GP_KEY_ESC:
			case GP_KEY_Q:
				gp_backend_exit(backend);
				return 1;
			break;
			}
		break;
		case GP_EV_SYS:
			switch (ev->code) {
			case GP_EV_SYS_RESIZE:
				gp_backend_resize_ack(backend);
				redraw(backend->pixmap);
				gp_backend_flip(backend);
			break;
			case GP_EV_SYS_QUIT:
				gp_backend_exit(backend);
				return 1;
			break;
			}
		break;
		}

		printf("-----------------------------\n");
	}

	return 0;
}

int main(void)
{
	gp_backend *win_1, *win_2;

	win_1 = gp_x11_init(NULL, 0, 0, 300, 300, "win 1", 0);
	win_2 = gp_x11_init(NULL, 0, 0, 300, 300, "win 2", 0);

	if (win_1 == NULL || win_2 == NULL) {
		gp_backend_exit(win_1);
		gp_backend_exit(win_2);
		return 1;
	}

	/* Update the backend screen */
	redraw(win_1->pixmap);
	redraw(win_2->pixmap);

	gp_backend_flip(win_1);
	gp_backend_flip(win_2);

	for (;;) {
		/*
		 * Wait for backend event.
		 *
		 * Either window is fine as they share connection.
		 */
		gp_backend *b = win_1 ? win_1 : win_2;

		if (b == NULL)
			return 0;

		gp_backend_wait(b);

		if (ev_loop(win_1, "win 1"))
			win_1 = NULL;

		if (ev_loop(win_2, "win 2"))
			win_2 = NULL;
	}

	gp_backend_exit(win_1);
	gp_backend_exit(win_2);

	return 0;
}
