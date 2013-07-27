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
 * Copyright (C) 2009-2013 Cyril Hrubis <metan@ucw.cz>                       *
 *                                                                           *
 *****************************************************************************/

 /*

   Simple V4L2 grabber interactive example.

  */

#include <errno.h>
#include <string.h>

#include <GP.h>

int main(int argc, char *argv[])
{
	GP_Backend *backend;
	GP_Grabber *grabber;
	const char *v4l2_device = "/dev/video0";
	unsigned int w = 320, h = 240;
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
			GP_SetDebugLevel(atoi(optarg));
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

	grabber = GP_GrabberV4L2Init(v4l2_device, w, h);

	if (grabber == NULL) {
		fprintf(stderr, "Failed to initalize grabber '%s': %s\n",
		        v4l2_device, strerror(errno));
		return 1;
	}

	backend = GP_BackendX11Init(NULL, 0, 0, grabber->frame->w,
	                            grabber->frame->h, "V4L2", 0);

	if (backend == NULL) {
		GP_GrabberExit(grabber);
		return 1;
	}

	if (GP_GrabberStart(grabber)) {
		fprintf(stderr, "Failed to start grabber\n");
		GP_BackendExit(backend);
		GP_GrabberExit(grabber);
		return 1;
	}

	printf("Press SPACE to change mode and Q to exit.\n");

	for (;;) {
		if (GP_GrabberPoll(grabber) > 0) {
			GP_Context *res, *img = grabber->frame;

			switch (mode) {
			case 0:
				res = img;
			break;
			case 1:
				GP_FilterEdgePrewitt(img, &res, NULL, NULL);
			//	GP_FilterEdgeSobel(img, &res, NULL, NULL);
			break;
			case 2:
				GP_FilterGaussianBlur(img, img, 1, 1, NULL);
				res = GP_FilterFloydSteinberg_RGB888_Alloc(img, GP_PIXEL_G2, NULL);
			break;
			}

			GP_Blit_Clipped(res, 0, 0, res->w, res->h, backend->context, 0, 0);
			GP_BackendFlip(backend);

			if (mode)
				GP_ContextFree(res);
		}

		usleep(1000);

		GP_BackendPoll(backend);

		/* Read and parse events */
		GP_Event ev;

		while (GP_BackendGetEvent(backend, &ev)) {
			switch (ev.type) {
			case GP_EV_KEY:

				/* ignore key up events */
				if (!ev.code)
					continue;

				switch (ev.val.key.key) {
				case GP_KEY_ESC:
				case GP_KEY_Q:
					GP_BackendExit(backend);
					GP_GrabberExit(grabber);
					return 0;
				break;
				case GP_KEY_SPACE:

					mode++;

					if (mode > 2)
						mode = 0;
				break;
				}
			}
		}
	}

	GP_BackendExit(backend);

	return 0;
}
