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
 * Copyright (C) 2009-2011 Cyril Hrubis <metan@ucw.cz>                       *
 *                                                                           *
 *****************************************************************************/

 /*

   Simple framebuffer image viewer.

  */

#include <signal.h>
#include <unistd.h>

#include <GP.h>
#include <backends/GP_Framebuffer.h>
#include <input/GP_InputDriverLinux.h>

static GP_Pixel black_pixel;
static GP_Pixel white_pixel;


static GP_Framebuffer *fb = NULL;

static void sighandler(int signo __attribute__((unused)))
{
	if (fb != NULL)
		GP_FramebufferExit(fb);

	exit(1);
}

static float calc_img_size(uint32_t img_w, uint32_t img_h,
                           uint32_t src_w, uint32_t src_h)
{
	float w_rat = 1.00 * src_w / img_w;
	float h_rat = 1.00 * src_h / img_h;

	return GP_MIN(w_rat, h_rat);
}

static GP_Context *image_to_display(GP_Context *img, uint32_t w, uint32_t h)
{
	float rat = calc_img_size(img->w, img->h, w, h);

//	GP_FilterGaussianBlur(img, img, 1, 1, NULL);

	return GP_FilterResize(img, NULL, GP_INTERP_CUBIC, img->w * rat, img->h * rat, NULL);
}

static int show_image(GP_Framebuffer *fb, const char *img_path, int clear)
{
	GP_Context *img;

	if (clear) {
		char buf[100];
		snprintf(buf, sizeof(buf), "Loading '%s'", img_path);
		GP_Fill(&fb->context, black_pixel);
		GP_BoxCenteredText(&fb->context, NULL, 0, 0,
                                   fb->context.w, fb->context.h,
                                   buf, white_pixel);
	}

	if (GP_LoadImage(img_path, &img) == 0) {
		GP_Context *img2 = image_to_display(img, fb->context.w, fb->context.h);

		GP_ContextFree(img);

		if (img2 == NULL)
			return 1;
		
		uint32_t cx = (fb->context.w - img2->w)/2;
		uint32_t cy = (fb->context.h - img2->h)/2;

		GP_Fill(&fb->context, black_pixel);

		GP_Blit(img2, 0, 0, img2->w, img2->h, &fb->context, cx, cy);

		GP_ContextFree(img2);
	}

	return 0;
}

int main(int argc, char *argv[])
{
	GP_InputDriverLinux *drv = NULL;
	char *input_dev = NULL;
	int sleep_sec = 0;

	int opt;

	while ((opt = getopt(argc, argv, "i:s:")) != -1) {
		switch (opt) {
		case 'i':
			input_dev = optarg;
		break;
		case 's':
			sleep_sec = atoi(optarg);
		break;
		default:
			fprintf(stderr, "Invalid paramter '%c'\n", opt);
		}
	}
	
	GP_SetDebugLevel(10);

	if (input_dev == NULL) {
		sleep_sec = 1;
	} else {
		drv = GP_InputDriverLinuxOpen(input_dev);
	
		if (drv == NULL) {
			fprintf(stderr, "Failed to initalize input device '%s'\n",
                	                input_dev);
			return 1;
		}
	}

	signal(SIGINT, sighandler);

	fb = GP_FramebufferInit("/dev/fb0");

	if (fb == NULL) {
		fprintf(stderr, "Failed to initalize framebuffer\n");
		return 1;
	}
	
	GP_EventSetScreenSize(fb->context.w, fb->context.h);
	
	black_pixel = GP_ColorToContextPixel(GP_COL_BLACK, &fb->context);
	white_pixel = GP_ColorToContextPixel(GP_COL_WHITE, &fb->context);

	int argf = optind;
	int argn = argf;

	show_image(fb, argv[argf], 1);

	/* Initalize select */
	fd_set rfds;
	FD_ZERO(&rfds);
	FD_SET(drv->fd, &rfds);
	struct timeval tv = {.tv_sec = sleep_sec, .tv_usec = 0};
	struct timeval *tvp = sleep_sec ? &tv : NULL;

	for (;;) {
		int ret;

		if (drv != NULL) {
			ret = select(drv->fd + 1, &rfds, NULL, NULL, tvp);
		
			tv.tv_sec = sleep_sec;
	
			switch (ret) {
			case -1:
				GP_FramebufferExit(fb);
				return 0;
			break;
			case 0:
				argn++;
				if (argn >= argc)
					argn = argf;
			
				show_image(fb, argv[argn], 0);
			break;
			default:
				while (GP_InputDriverLinuxRead(drv));
			}

			FD_SET(drv->fd, &rfds);
		} else {
			sleep(sleep_sec);
			
			argn++;
			if (argn >= argc)
				argn = argf;
			
			show_image(fb, argv[argn], 0);
		}

		/* Read and parse events */
		GP_Event ev;

		while (GP_EventGet(&ev)) {

			GP_EventDump(&ev);
			
			switch (ev.type) {
			case GP_EV_KEY:
				if (ev.code != GP_EV_KEY_DOWN)
					continue;

				switch (ev.val.key.key) {
				case GP_KEY_ENTER:
					GP_FramebufferExit(fb);
					return 0;
				break;
				case GP_KEY_ESC:
				case GP_KEY_SPACE:
				case GP_KEY_Q:
					argn++;
					if (argn >= argc)
						argn = argf;
					show_image(fb, argv[argn], 1);
				break;
				}
			break;
			}
		}
	}

	GP_FramebufferExit(fb);

	return 0;
}
