// SPDX-License-Identifier: GPL-2.1-or-later
/*
 * Copyright (C) 2009-2013 Cyril Hrubis <metan@ucw.cz>
 */

 /*

   Simple V4L2 grabber interactive example.

  */

#include <errno.h>
#include <string.h>
#include <stdio.h>

#include <gfxprim.h>

static void save_image(gp_pixmap *p)
{
	static unsigned int counter;
	char fname[256];

	snprintf(fname, sizeof(fname), "screenshot%u.jpg", counter++);

	gp_save_jpg(p, fname, NULL);
}

int main(int argc, char *argv[])
{
	gp_backend *backend;
	gp_grabber *grabber;
	const char *v4l2_device = "/dev/video0";
	unsigned int w = 640, h = 480;
	int mode = 0;
	int opt;

	while ((opt = getopt(argc, argv, "d:hH:W:l:")) != -1) {
		switch (opt) {
		case 'd':
			v4l2_device = optarg;
		break;
		case 'W':
			w = atoi(optarg);
		break;
		case 'H':
			h = atoi(optarg);
		break;
		case 'l':
			gp_set_debug_level(atoi(optarg));
		break;
		case 'h':
			printf("Usage; %s opts\n", argv[0]);
			printf("-d  v4l2 device name (default is '/dev/video0'\n"
			       "-W  output image width, default is 640\n"
			       "-H  output image height, default is 480\n"
			       "-l  sets GFXprim debug level (default is 0)\n"
			       "-h  prints this help\n");
			return 0;
		break;
		default:
			fprintf(stderr, "Invalid paramter '%c'\n", opt);
			return 1;
		}
	}

	grabber = gp_grabber_v4l2_init(v4l2_device, w, h);

	if (grabber == NULL) {
		fprintf(stderr, "Failed to initalize grabber '%s': %s\n",
		        v4l2_device, strerror(errno));
		return 1;
	}

	backend = gp_x11_init(NULL, 0, 0, grabber->frame->w,
	                            grabber->frame->h, "V4L2", 0);

	if (backend == NULL) {
		gp_grabber_exit(grabber);
		return 1;
	}

	if (gp_grabber_start(grabber)) {
		fprintf(stderr, "Failed to start grabber\n");
		gp_backend_exit(backend);
		gp_grabber_exit(grabber);
		return 1;
	}

	printf("Press SPACE to change mode and Q to exit.\n");

	for (;;) {
		if (gp_grabber_poll(grabber) > 0) {
			gp_pixmap *res, *img = grabber->frame;

			switch (mode) {
			case 0:
				res = img;
			break;
			case 1:
			//	gp_filter_edge_prewitt(img, &res, NULL, NULL);
				gp_filter_edge_sobel(img, &res, NULL, NULL);
			break;
			case 2:
				gp_filter_gaussian_blur(img, img, 1, 1, NULL);
				res = gp_filter_floyd_steinberg_alloc(img, GP_PIXEL_G2, NULL);
			break;
			}

			unsigned int c_x = (backend->pixmap->w - res->w) / 2;
			unsigned int c_y = (backend->pixmap->h - res->h) / 2;

			gp_blit_clipped(res, 0, 0, res->w, res->h, backend->pixmap, c_x, c_y);
			gp_backend_flip(backend);

			if (mode)
				gp_pixmap_free(res);
		}

		usleep(1000);

		gp_backend_poll(backend);

		/* Read and parse events */
		gp_event ev;

		while (gp_backend_get_event(backend, &ev)) {
			switch (ev.type) {
			case GP_EV_KEY:

				/* ignore key up events */
				if (!ev.code)
					continue;

				switch (ev.val.key.key) {
				case GP_KEY_ESC:
				case GP_KEY_Q:
					gp_backend_exit(backend);
					gp_grabber_exit(grabber);
					return 0;
				break;
				case GP_KEY_S:
					save_image(backend->pixmap);
				break;
				case GP_KEY_SPACE:

					mode++;

					if (mode > 2)
						mode = 0;
				break;
				}
			break;
			case GP_EV_SYS:
				if (ev.code == GP_EV_SYS_RESIZE) {
					gp_backend_resize_ack(backend);
					gp_fill(backend->pixmap, 0);
				}
			break;
			}
		}
	}

	gp_backend_exit(backend);

	return 0;
}
