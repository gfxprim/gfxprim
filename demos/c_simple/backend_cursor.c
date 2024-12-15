// SPDX-License-Identifier: GPL-2.1-or-later
/*
 * Copyright (C) 2009-2024 Cyril Hrubis <metan@ucw.cz>
 */

 /*

   Simple backend cursor example.

  */

#include <stdio.h>
#include <gfxprim.h>

static void redraw(gp_backend *self)
{
	gp_pixmap *pixmap = self->pixmap;
	gp_pixel white_pixel, black_pixel;

	gp_text_style font = {
		.pixel_xmul = 2,
		.pixel_ymul = 2,
		.pixel_xspace = 1,
		.pixel_yspace = 1,
	};

	font.font = gp_font_face_lookup("haxor-narrow-15", GP_FONT_REGULAR | GP_FONT_FALLBACK);

	black_pixel = gp_rgb_to_pixmap_pixel(0x00, 0x00, 0x00, pixmap);
	white_pixel = gp_rgb_to_pixmap_pixel(0xff, 0xff, 0xff, pixmap);

	gp_fill(pixmap, black_pixel);

	gp_size text_h = gp_text_height(&font);

	gp_text(pixmap, &font, gp_pixmap_w(pixmap)/2, gp_pixmap_h(pixmap)/2-text_h, GP_VALIGN_CENTER|GP_ALIGN_CENTER,
		white_pixel, black_pixel, "Press C to change cursor");

	gp_text(pixmap, &font, gp_pixmap_w(pixmap)/2, gp_pixmap_h(pixmap)/2, GP_VALIGN_CENTER|GP_ALIGN_CENTER,
		white_pixel, black_pixel, "Press H to hide cursor");

	gp_text(pixmap, &font, gp_pixmap_w(pixmap)/2, gp_pixmap_h(pixmap)/2+text_h, GP_VALIGN_CENTER|GP_ALIGN_CENTER,
		white_pixel, black_pixel, "Press S to show cursor");

	gp_backend_flip(self);
}

static void change_cursor(gp_backend *self)
{
	static enum gp_backend_cursors cursor = GP_BACKEND_CURSOR_ARROW;

	cursor = (cursor+1) % GP_BACKEND_CURSOR_MAX;
	gp_backend_cursor_set(self, cursor);
}

int main(int argc, char *argv[])
{
	gp_backend *backend;
	const char *backend_opts = NULL;
	int opt;

	while ((opt = getopt(argc, argv, "b:h")) != -1) {
		switch (opt) {
		case 'b':
			backend_opts = optarg;
		break;
		case 'h':
			gp_backend_init_help();
			return 0;
		break;
		default:
			fprintf(stderr, "Invalid paramter '%c'\n", opt);
			return 1;
		}
	}

	backend = gp_backend_init(backend_opts, 100, 100, "Cursor Example");
	if (!backend) {
		fprintf(stderr, "Failed to initialize backend\n");
		return 1;
	}

	redraw(backend);

	/* Handle events */
	for (;;) {
		gp_event *ev = gp_backend_ev_wait(backend);

		gp_ev_dump(ev);

		switch (ev->type) {
		case GP_EV_KEY:
			if (ev->code != GP_EV_KEY_DOWN)
				continue;

			switch (ev->val) {
			case GP_KEY_ESC:
			case GP_KEY_Q:
				gp_backend_exit(backend);
				return 0;
			break;
			case GP_KEY_C:
				change_cursor(backend);
			break;
			case GP_KEY_H:
				gp_backend_cursor_set(backend, GP_BACKEND_CURSOR_HIDE);
			break;
			case GP_KEY_S:
				gp_backend_cursor_set(backend, GP_BACKEND_CURSOR_SHOW);
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
