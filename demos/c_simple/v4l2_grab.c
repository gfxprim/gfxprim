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
 * Copyright (C) 2009-2012 Cyril Hrubis <metan@ucw.cz>                       *
 *                                                                           *
 *****************************************************************************/

 /*

   Gets an image from v4l2 device.

  */

#include <string.h>
#include <errno.h>

#include <GP.h>

static int get_image(const char *filename, GP_Grabber *grabber)
{
	/* turn on grabber */
	if (GP_GrabberStart(grabber)) {
		fprintf(stderr, "Failed to start grabber\n");
		return 1;
	}

	/* throw away first frame, it's usually wrong */
	while (!GP_GrabberPoll(grabber))
		usleep(100000);
	
	while (!GP_GrabberPoll(grabber))
		usleep(100000);

	/* save image */
	if (GP_SaveJPG(grabber->frame, filename, NULL)) {
		fprintf(stderr, "Failed to save image '%s': %s",
		        filename, strerror(errno));
		return 1;
	}
	
	/* turn off grabber */
	if (GP_GrabberStop(grabber)) {
		fprintf(stderr, "Failed to start grabber\n");
		return 1;
	}

	return 0;
}

int main(int argc, char *argv[])
{
	const char *v4l2_device = "/dev/video0";
	const char *image_filename = "frame.jpg";
	unsigned int w = 640, h = 480;
	int secs = 0;
	int opt;
	
	while ((opt = getopt(argc, argv, "d:hH:o:W:l:s:")) != -1) {
		switch (opt) {
		case 'o':
			image_filename = optarg;
		break;
		case 'd':
			v4l2_device = optarg;
		break;
		case 'W':
			w = atoi(optarg);
		break;
		case 'H':
			h = atoi(optarg);
		break;
		case 's':
			secs = atoi(optarg);
		break;
		case 'l':
			GP_SetDebugLevel(atoi(optarg));
		break;
		case 'h':
			printf("Usage; %s opts\n", argv[0]);
			printf("-o  output image file, default is 'frame.jpg'\n"
			       "-d  v4l2 device name (default is '/dev/video0'\n"
			       "-W  output image width, default is 640\n"
			       "-H  output image height, default is 480\n"
			       "-l  sets GFXprim debug level (default is 0)\n"
			       "-s  take image every s seconds (the images are stored as frameX.jpg)\n"
			       "-h  prints this help\n");
			return 0;
		break;
		default:
			fprintf(stderr, "Invalid paramter '%c'\n", opt);
			return 1;
		}
	}
	
	GP_Grabber *grabber = GP_GrabberV4L2Init(v4l2_device, w, h);

	if (grabber == NULL) {
		fprintf(stderr, "Failed to initalize grabber '%s': %s\n",
		        v4l2_device, strerror(errno));
		return 1;
	}

	if (secs == 0) {
		get_image(image_filename, grabber);
		GP_GrabberExit(grabber);
		return 0;
	}

	int i = 0;
	
	for (;;) {
		char buf[128];

		snprintf(buf, sizeof(buf), "frame%03i.jpg", i++);
		
		if (get_image(buf, grabber)) {
			fprintf(stderr, "Failed to get image, exitting...\n");
			return 1;
		}

		sleep(secs);
	}


	return 0;
}
