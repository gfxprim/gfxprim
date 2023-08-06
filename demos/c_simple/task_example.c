// SPDX-License-Identifier: GPL-2.1-or-later
/*
 * Copyright (C) 2009-2021 Cyril Hrubis <metan@ucw.cz>
 */

 /*

   Simple backend task example.

  */

#include <stdio.h>
#include <gfxprim.h>

static void redraw(gp_backend *self)
{
	gp_pixmap *pixmap = self->pixmap;
	gp_pixel white_pixel, black_pixel;

	black_pixel = gp_rgb_to_pixmap_pixel(0x00, 0x00, 0x00, pixmap);
	white_pixel = gp_rgb_to_pixmap_pixel(0xff, 0xff, 0xff, pixmap);

	gp_fill(pixmap, black_pixel);
	gp_line(pixmap, 0, 0, pixmap->w - 1, pixmap->h - 1, white_pixel);
	gp_line(pixmap, 0, pixmap->h - 1, pixmap->w - 1, 0, white_pixel);

	gp_backend_flip(self);
}

static int redraw_task_callback(gp_task *self)
{
	gp_backend *backend = self->priv;
	gp_pixel red_pixel;

	red_pixel = gp_rgb_to_pixmap_pixel(0xff, 0x00, 0x00, backend->pixmap);

	gp_text(backend->pixmap, NULL, backend->pixmap->w/2, 10,
	        GP_ALIGN_CENTER|GP_VALIGN_BELOW,
		red_pixel, 0, "Task called!");

	gp_backend_flip(backend);

	return 0;
}

int main(int argc, char *argv[])
{
	gp_backend *backend;
	static gp_task_queue task_queue = {};
	const char *backend_opts = "X11:100x100";
	int opt;

	while ((opt = getopt(argc, argv, "b:h")) != -1) {
		switch (opt) {
		case 'b':
			backend_opts = optarg;
		break;
		case 'h':
			gp_backend_init(NULL, NULL);
			return 0;
		break;
		default:
			fprintf(stderr, "Invalid paramter '%c'\n", opt);
			return 1;
		}
	}

	backend = gp_backend_init(backend_opts, "Backend Example");
	if (!backend) {
		fprintf(stderr, "Failed to initialize backend\n");
		return 1;
	}

	redraw(backend);

	gp_backend_task_queue_set(backend, &task_queue);

	gp_task redraw_task = {
		.id = "redraw task",
		.callback = redraw_task_callback,
		.priv = backend,
		.prio = GP_TASK_MIN_PRIO,
	};

	/* Handle events */
	for (;;) {
		gp_event *ev = gp_backend_wait_event(backend);

		switch (ev->type) {
		case GP_EV_KEY:
			switch (ev->val) {
			case GP_KEY_ESC:
			case GP_KEY_Q:
				gp_backend_exit(backend);
				return 0;
			break;
			case GP_KEY_T:
				gp_backend_task_ins(backend, &redraw_task);
			break;
			}
		break;
		case GP_EV_SYS:
			switch (ev->code) {
			case GP_EV_SYS_RESIZE:
				gp_backend_resize_ack(backend);
				redraw(backend);
			break;
			case GP_EV_SYS_QUIT:
				gp_backend_exit(backend);
				return 0;
			break;
			}
		break;
		}
	}

	gp_backend_exit(backend);

	return 0;
}
