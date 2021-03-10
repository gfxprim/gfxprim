// SPDX-License-Identifier: GPL-2.1-or-later
/*
 * Copyright (C) 2009-2013 Cyril Hrubis <metan@ucw.cz>
 */

 /*

   Simple virtual backend test.

   Virtual backned allows you to test interactively pixel types that your
   hardware/xserver doesn't support.

  */

#include <gfxprim.h>

static gp_pixel white_pixel, black_pixel, red_pixel, blue_pixel, green_pixel;

static void redraw(gp_backend *backend)
{
	gp_pixmap *pixmap = backend->pixmap;

	/* Now draw some testing patters */
	black_pixel = gp_rgb_to_pixmap_pixel(0x00, 0x00, 0x00, pixmap);
	white_pixel = gp_rgb_to_pixmap_pixel(0xff, 0xff, 0xff, pixmap);
	red_pixel   = gp_rgb_to_pixmap_pixel(0xff, 0x00, 0x00, pixmap);
	blue_pixel  = gp_rgb_to_pixmap_pixel(0x00, 0x00, 0xff, pixmap);
	green_pixel = gp_rgb_to_pixmap_pixel(0x00, 0xff, 0x00, pixmap);

	gp_fill(pixmap, white_pixel);

	unsigned int i, j;
	for (i = 0; i < 40; i++) {
		gp_hline_xyw(pixmap, 0, i, i, black_pixel);
		gp_hline_xyw(pixmap, 1, i + 40, i, black_pixel);
		gp_hline_xyw(pixmap, 2, i + 80, i, black_pixel);
		gp_hline_xyw(pixmap, 3, i + 120, i, black_pixel);
		gp_hline_xyw(pixmap, 4, i + 160, i, black_pixel);
		gp_hline_xyw(pixmap, 5, i + 200, i, black_pixel);
		gp_hline_xyw(pixmap, 6, i + 240, i, black_pixel);
		gp_hline_xyw(pixmap, 7, i + 280, i, black_pixel);
	}

	for (i = 0; i < 256; i++) {
		for (j = 0; j < 256; j++) {
			uint8_t val = 1.00 * sqrt(i*i + j*j)/sqrt(2) + 0.5;

			gp_pixel pix = gp_rgb_to_pixmap_pixel(i, j, val, pixmap);
			gp_putpixel(pixmap, i + 60, j + 10, pix);
		}
	}

	gp_text(pixmap, NULL, 60, 270, GP_VALIGN_BELOW|GP_ALIGN_RIGHT,
	        black_pixel, white_pixel, "Lorem Ipsum dolor sit...");

	gp_text(pixmap, NULL, 60, 290, GP_VALIGN_BELOW|GP_ALIGN_RIGHT,
	        red_pixel, white_pixel, "Lorem Ipsum dolor sit...");

	gp_text(pixmap, NULL, 60, 310, GP_VALIGN_BELOW|GP_ALIGN_RIGHT,
	        green_pixel, white_pixel, "Lorem Ipsum dolor sit...");

	gp_text(pixmap, NULL, 60, 330, GP_VALIGN_BELOW|GP_ALIGN_RIGHT,
	        blue_pixel, white_pixel, "Lorem Ipsum dolor sit...");

	/* Update the backend screen */
	gp_backend_flip(backend);
}

int main(int argc, char *argv[])
{
	gp_backend *backend;
	const char *backend_opts = "X11:350x350";
	int opt;
	gp_pixel_type emul_type = GP_PIXEL_UNKNOWN;

	while ((opt = getopt(argc, argv, "b:h:p:")) != -1) {
		switch (opt) {
		case 'b':
			backend_opts = optarg;
		break;
		case 'p':
			emul_type = gp_pixel_type_by_name(optarg);

			if (emul_type == GP_PIXEL_UNKNOWN) {
				fprintf(stderr, "Invalid pixel type '%s'\n", optarg);
				return 1;
                        }
		break;
		case 'h':
			gp_backend_init("help", NULL);
			return 0;
		break;
		default:
			fprintf(stderr, "Invalid paramter '%c'\n", opt);
			return 1;
		}
	}

	/* Turn on debug messages */
	gp_set_debug_level(10);

	backend = gp_backend_init(backend_opts, "Virtual Backend Example");

	if (emul_type != GP_PIXEL_UNKNOWN) {
		gp_backend *emul;

		/*
		 * Create an emulated backend on the top of real backend.
		 *
		 * The GP_BACKEND_CALL_EXIT says that when calling exit on
		 * emulated backend, the real backend exit will be called as
		 * well.
		 */
		emul = gp_backend_virt_init(backend, emul_type, GP_BACKEND_CALL_EXIT);

		if (emul == NULL) {
			fprintf(stderr, "Failed to create Virtual Backend\n");
			gp_backend_exit(backend);
			return 1;
		}

		/* Once created virtual backend behaves exactly like a real one */
		backend = emul;
	}

	redraw(backend);

	for (;;) {
		if (backend->poll)
			gp_backend_poll(backend);

		usleep(1000);

		/* Read and parse events */
		gp_event *ev;

		while ((ev = gp_backend_get_event(backend))) {

			gp_event_dump(ev);

			switch (ev->type) {
			case GP_EV_KEY:
				switch (ev->key.key) {
				case GP_KEY_ESC:
				case GP_KEY_Q:
					gp_backend_exit(backend);
					return 0;
				break;
				}
			break;
			case GP_EV_SYS:
				switch(ev->code) {
				case GP_EV_SYS_RESIZE:
					gp_backend_resize_ack(backend);
					redraw(backend);
				break;
				}
			break;
			}
		}
	}

	gp_backend_exit(backend);

	return 0;
}
