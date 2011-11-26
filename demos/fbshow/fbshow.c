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
static int show_progress = 0;

static int image_loader_callback(GP_ProgressCallback *self)
{
	static GP_Size size = 0;

	if (abort_flag)
		return 1;

	if (!show_progress)
		return 0;

	char buf[100];

	snprintf(buf, sizeof(buf), "%s ... %-3.1f%%",
	         (const char*)self->priv, self->percentage);

	GP_Context *c = &fb->context;

	int align = GP_ALIGN_CENTER|GP_VALIGN_ABOVE;

	GP_TextClear(c, NULL, c->w/2, c->h - 4, align,
	             black_pixel, GP_MAX(size, GP_TextWidth(NULL, buf)));

	GP_Text(c, NULL, c->w/2, c->h - 4, align,
	        white_pixel, black_pixel, buf);

	size = GP_TextWidth(NULL, buf);

	return 0;
}

struct loader_params {
	const char *img_path;
	int show_progress;
	int show_progress_once;
	int show_info;
};

static float calc_img_size(uint32_t img_w, uint32_t img_h,
                           uint32_t src_w, uint32_t src_h)
{
	float w_rat = 1.00 * src_w / img_w;
	float h_rat = 1.00 * src_h / img_h;

	return GP_MIN(w_rat, h_rat);
}

static const char *img_name(const char *img_path)
{
	int i, len = strlen(img_path);

	for (i = len - 1; i > 0; i--) {
		if (img_path[i] == '/')
			return &img_path[i+1];
	}

	return NULL;
}

static void *image_loader(void *ptr)
{
	struct loader_params *params = ptr;
	GP_ProgressCallback callback = {.callback = image_loader_callback};

	show_progress = params->show_progress || params->show_progress_once;
	params->show_progress_once = 0;

	fprintf(stderr, "Loading '%s'\n", params->img_path);

	GP_Context *img = NULL;

	callback.priv = "Loading image";
	
	if (GP_LoadImage(params->img_path, &img, &callback) != 0) {
		GP_Fill(&fb->context, black_pixel);
		GP_Text(&fb->context, NULL, fb->context.w/2, fb->context.h/2,
		        GP_ALIGN_CENTER|GP_VALIGN_CENTER, black_pixel, white_pixel,
			"Failed to load image :(");
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

	w = img->w;
	h = img->h;

	/* Workaround */
	if (img->pixel_type != GP_PIXEL_RGB888) {
		GP_Context *tmp = GP_ContextConvert(img, NULL, GP_PIXEL_RGB888);
		GP_ContextFree(img);
		img = tmp;
	}
	
	GP_Context *ret;

	//callback.priv = "Blurring Image";
	//if (GP_FilterGaussianBlur(img, img, 0.25/rat, 0.25/rat, &callback) == NULL)
	//	return NULL;
	
	callback.priv = "Resampling Image";
	ret = GP_FilterResize(img, NULL, GP_INTERP_CUBIC, img->w * rat, img->h * rat, &callback);
	GP_ContextFree(img);

	if (ret == NULL)
		return NULL;

	switch (rotate) {
	case 0:
	break;
	case 90:
		callback.priv = "Rotating image (90)";
		img = GP_FilterRotate90(ret, NULL, &callback);
	break;
	case 180:
		callback.priv = "Rotating image (180)";
		img = GP_FilterRotate180(ret, NULL, &callback);
	break;
	case 270:
		callback.priv = "Rotating image (270)";
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

	if (!params->show_info)
		return NULL;

	GP_Size th = GP_TextHeight(NULL);
	
	GP_Print(&fb->context, NULL, 11, 11, GP_ALIGN_RIGHT|GP_VALIGN_BOTTOM,
	         black_pixel, white_pixel, "%ux%u", w, h);
	
	GP_Print(&fb->context, NULL, 10, 10, GP_ALIGN_RIGHT|GP_VALIGN_BOTTOM,
	         white_pixel, black_pixel, "%ux%u", w, h);
	
	GP_Print(&fb->context, NULL, 11, 13 + th, GP_ALIGN_RIGHT|GP_VALIGN_BOTTOM,
	         black_pixel, white_pixel, "1:%3.3f", rat);
	
	GP_Print(&fb->context, NULL, 10, 12 + th, GP_ALIGN_RIGHT|GP_VALIGN_BOTTOM,
	         white_pixel, black_pixel, "1:%3.3f", rat);
	
	GP_Print(&fb->context, NULL, 11, 15 + 2 * th, GP_ALIGN_RIGHT|GP_VALIGN_BOTTOM,
	         black_pixel, white_pixel, "%s", img_name(params->img_path));
	
	GP_Print(&fb->context, NULL, 10, 14 + 2 * th, GP_ALIGN_RIGHT|GP_VALIGN_BOTTOM,
	         white_pixel, black_pixel, "%s", img_name(params->img_path));

	return NULL;
}

static pthread_t loader_thread = (pthread_t)0;

static void show_image(struct loader_params *params)
{
	int ret;
	
	/* stop previous loader thread */
	if (loader_thread) {
		abort_flag = 1;
		pthread_join(loader_thread, NULL);
		loader_thread = (pthread_t)0;
		abort_flag = 0;
	}

	ret = pthread_create(&loader_thread, NULL, image_loader, (void*)params);

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
	struct loader_params params = {NULL, 0, 0, 0};
	int opt;

	while ((opt = getopt(argc, argv, "Ii:Ps:r:")) != -1) {
		switch (opt) {
		case 'I':
			params.show_info = 1;
		break;
		case 'P':
			params.show_progress = 1;
		break;
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

	GP_Fill(&fb->context, black_pixel);

	int argf = optind;
	int argn = argf;

	params.show_progress_once = 1;
	params.img_path = argv[argf];
	show_image(&params);

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
			
				params.img_path = argv[argn];
				show_image(&params);
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
			
			params.img_path = argv[argn];
			show_image(&params);
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
				case GP_KEY_I:
					params.show_info = !params.show_info;
					
					params.show_progress_once = 1;
					show_image(&params);
				break;
				case GP_KEY_P:
					params.show_progress = !params.show_progress;
				break;
				case GP_KEY_R:
					rotate += 90;
					if (rotate > 270)
						rotate = 0;
					
					params.show_progress_once = 1;
					show_image(&params);
				break;
				case GP_KEY_ESC:
				case GP_KEY_ENTER:
				case GP_KEY_Q:
					GP_FramebufferExit(fb);
					return 0;
				break;
				case GP_KEY_RIGHT:
				case GP_KEY_UP:
				case GP_KEY_SPACE:
					argn++;
					if (argn >= argc)
						argn = argf;
					
					params.show_progress_once = 1;
					params.img_path = argv[argn];
					show_image(&params);
				break;
				case GP_KEY_BACKSPACE:
				case GP_KEY_LEFT:
				case GP_KEY_DOWN:
					argn--;

					if (argn < argf)
						argn = argc - 1;

					params.show_progress_once = 1;
					params.img_path = argv[argn];
					show_image(&params);
				break;
				}
			break;
			}
		}
	}

	GP_FramebufferExit(fb);

	return 0;
}
