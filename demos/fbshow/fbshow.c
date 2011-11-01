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
#include <string.h>
#include <pthread.h>

#include <GP.h>
#include <backends/GP_Framebuffer.h>
#include <input/GP_InputDriverLinux.h>

static GP_Pixel black_pixel;
static GP_Pixel white_pixel;

static GP_Framebuffer *fb = NULL;

/* image loader thread */
static int abort_flag = 0;
static int rotate = 0;

static int image_loader_callback(GP_ProgressCallback *self)
{
	if (abort_flag)
		return 1;
	
	return 0;
}

struct loader_params {
	const char *img_path;
	int clear;
};

static float calc_img_size(uint32_t img_w, uint32_t img_h,
                           uint32_t src_w, uint32_t src_h)
{
	float w_rat = 1.00 * src_w / img_w;
	float h_rat = 1.00 * src_h / img_h;

	return GP_MIN(w_rat, h_rat);
}

static void *image_loader(void *ptr)
{
	struct loader_params *params = ptr;

	fprintf(stderr, "Loading '%s'\n", params->img_path);

	if (params->clear) {
		char buf[100];
		snprintf(buf, sizeof(buf), "Loading '%s'", params->img_path);
		GP_Fill(&fb->context, black_pixel);
		GP_BoxCenteredText(&fb->context, NULL, 0, 0,
                                   fb->context.w, fb->context.h,
                                   buf, white_pixel);
	}

	GP_Context *img = NULL;

	if (GP_LoadImage(params->img_path, &img) != 0) {
		GP_BoxCenteredText(&fb->context, NULL, 0, 0,
                                   fb->context.w, fb->context.h,
                                   "Failed to load image", white_pixel);
		return NULL;
	}

	GP_Size w, h;

	switch (rotate) {
	case 0:
	case 180:
	default:
		w = fb->context.w;
		h = fb->context.h;
	break;
	case 90:
	case 270:
		w = fb->context.h;
		h = fb->context.w;
	break;
	}

	float rat = calc_img_size(img->w, img->h, w, h);

	/* Workaround */
	if (img->pixel_type != GP_PIXEL_RGB888) {
		GP_Context *tmp = GP_ContextConvert(img, GP_PIXEL_RGB888);
		GP_ContextFree(img);
		img = tmp;
	}
	
	GP_ProgressCallback callback = {.callback = image_loader_callback};
	
	GP_Context *ret;

//	if (GP_FilterGaussianBlur(img, img, 0.7, 0.7, &callback))
//		return NULL;
	ret = GP_FilterResize(img, NULL, GP_INTERP_CUBIC, img->w * rat, img->h * rat, &callback);
	GP_ContextFree(img);

	if (ret == NULL)
		return NULL;

	switch (rotate) {
	case 0:
	break;
	case 90:
		img = GP_FilterRotate90(ret, NULL, &callback);
	break;
	case 180:
		img = GP_FilterRotate180(ret, NULL, &callback);
	break;
	case 270:
		img = GP_FilterRotate270(ret, NULL, &callback);
	break;
	}

	if (rotate) {
		GP_ContextFree(ret);
		ret = img;
	}

	if (img == NULL)
		return NULL;

	uint32_t cx = (fb->context.w - ret->w)/2;
	uint32_t cy = (fb->context.h - ret->h)/2;

	GP_Blit(ret, 0, 0, ret->w, ret->h, &fb->context, cx, cy);
	GP_ContextFree(ret);

	/* clean up the rest of the display */
	GP_FillRectXYWH(&fb->context, 0, 0, cx, fb->context.h, black_pixel);
	GP_FillRectXYWH(&fb->context, 0, 0, fb->context.w, cy, black_pixel);
	GP_FillRectXYWH(&fb->context, ret->w+cx, 0, cx, fb->context.h, black_pixel);
	GP_FillRectXYWH(&fb->context, 0, ret->h+cy, fb->context.w, cy, black_pixel);

	return NULL;
}

static pthread_t loader_thread = (pthread_t)0;
static struct loader_params params;

static void show_image(const char *img_path, int clear)
{
	int ret;
	
	params.img_path = img_path;
	params.clear = clear;

	/* stop previous loader thread */
	if (loader_thread) {
		abort_flag = 1;
		pthread_join(loader_thread, NULL);
		loader_thread = (pthread_t)0;
		abort_flag = 0;
	}

	ret = pthread_create(&loader_thread, NULL, image_loader, (void*)&params);

	if (ret) {
		fprintf(stderr, "Failed to start thread: %s\n", strerror(ret));
		GP_FramebufferExit(fb);
		exit(1);
	}
}

static void sighandler(int signo __attribute__((unused)))
{
	if (fb != NULL)
		GP_FramebufferExit(fb);

	exit(1);
}

int main(int argc, char *argv[])
{
	GP_InputDriverLinux *drv = NULL;
	char *input_dev = NULL;
	int sleep_sec = -1;

	int opt;

	while ((opt = getopt(argc, argv, "i:s:r:")) != -1) {
		switch (opt) {
		case 'i':
			input_dev = optarg;
		break;
		case 's':
			sleep_sec = atoi(optarg);
		break;
		case 'r':
			if (!strcmp(optarg, "90"))
				rotate = 90;
			else if (!strcmp(optarg, "180"))
				rotate = 180;
			else if (!strcmp(optarg, "270"))
				rotate = 270;
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
	signal(SIGSEGV, sighandler);
	signal(SIGBUS, sighandler);
	signal(SIGABRT, sighandler);

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

	show_image(argv[argf], 1);

	/* Initalize select */
	fd_set rfds;
	FD_ZERO(&rfds);
	FD_SET(drv->fd, &rfds);
	struct timeval tv = {.tv_sec = sleep_sec, .tv_usec = 0};
	struct timeval *tvp = sleep_sec != -1 ? &tv : NULL;

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
			
				show_image(argv[argn], 0);
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
			
			show_image(argv[argn], 0);
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
				case GP_KEY_R:
					rotate += 90;
					if (rotate > 270)
						rotate = 0;
					show_image(argv[argn], 1);
				break;
				case GP_KEY_ESC:
				case GP_KEY_ENTER:
				case GP_KEY_Q:
					GP_FramebufferExit(fb);
					return 0;
				break;
				case GP_KEY_UP:
				case GP_KEY_SPACE:
					argn++;
					if (argn >= argc)
						argn = argf;
					show_image(argv[argn], 1);
				break;
				case GP_KEY_DOWN:
					argn--;

					if (argn < argf)
						argn = argc - 1;

					show_image(argv[argn], 1);
				break;
				}
			break;
			}
		}
	}

	GP_FramebufferExit(fb);

	return 0;
}
