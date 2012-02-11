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

   Simple framebuffer image viewer.

  */

#include <signal.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>

#include <GP.h>
#include <backends/GP_Backends.h>
#include <input/GP_InputDriverLinux.h>

#include "cpu_timer.h"

static GP_Pixel black_pixel;
static GP_Pixel white_pixel;

static GP_Backend *backend = NULL;
static GP_Context *context = NULL;

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

	GP_Context *c = context;

	int align = GP_ALIGN_CENTER|GP_VALIGN_ABOVE;

	GP_TextClear(c, NULL, c->w/2, c->h - 4, align,
	             black_pixel, GP_MAX(size, GP_TextWidth(NULL, buf)));

	GP_Text(c, NULL, c->w/2, c->h - 4, align,
	        white_pixel, black_pixel, buf);

	size = GP_TextWidth(NULL, buf);

	GP_BackendFlip(backend);

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
	struct cpu_timer timer;
	struct cpu_timer sum_timer;

	cpu_timer_start(&sum_timer, "sum");

	show_progress = params->show_progress || params->show_progress_once;
	params->show_progress_once = 0;

	fprintf(stderr, "Loading '%s'\n", params->img_path);

	GP_Context *img = NULL;

	callback.priv = "Loading image";

	cpu_timer_start(&timer, "Loading");
	if (GP_LoadImage(params->img_path, &img, &callback) != 0) {
		GP_Fill(context, black_pixel);
		GP_Text(context, NULL, context->w/2, context->h/2,
		        GP_ALIGN_CENTER|GP_VALIGN_CENTER, black_pixel, white_pixel,
			"Failed to load image :(");
		return NULL;
	}
	cpu_timer_stop(&timer);

	GP_Size w, h;

	switch (rotate) {
	case 0:
	case 180:
	default:
		w = context->w;
		h = context->h;
	break;
	case 90:
	case 270:
		w = context->h;
		h = context->w;
	break;
	}

	float rat = calc_img_size(img->w, img->h, w, h);

	w = img->w;
	h = img->h;

	/* Workaround */
	if (img->pixel_type != GP_PIXEL_RGB888) {
		GP_Context *tmp = GP_ContextConvert(img, GP_PIXEL_RGB888);
		GP_ContextFree(img);
		img = tmp;
	}
	
	GP_Context *ret;

/*	if (rat < 1) {
		cpu_timer_start(&timer, "Blur");
		callback.priv = "Blurring Image";
		if (GP_FilterGaussianBlur(img, img, 0.5/rat, 0.5/rat, &callback) == NULL)
			return NULL;
		cpu_timer_stop(&timer);
	} */

	cpu_timer_start(&timer, "Resampling");
	callback.priv = "Resampling Image";
	ret = GP_FilterResize(img, NULL, GP_INTERP_LINEAR_LF_INT, img->w * rat, img->h * rat, &callback);
	GP_ContextFree(img);
	cpu_timer_stop(&timer);

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

	uint32_t cx = (context->w - ret->w)/2;
	uint32_t cy = (context->h - ret->h)/2;

	cpu_timer_start(&timer, "Blitting");
	GP_Blit_Raw(ret, 0, 0, ret->w, ret->h, context, cx, cy);
	cpu_timer_stop(&timer);
	GP_ContextFree(ret);

	/* clean up the rest of the display */
	GP_FillRectXYWH(context, 0, 0, cx, context->h, black_pixel);
	GP_FillRectXYWH(context, 0, 0, context->w, cy, black_pixel);
	GP_FillRectXYWH(context, ret->w+cx, 0, cx, context->h, black_pixel);
	GP_FillRectXYWH(context, 0, ret->h+cy, context->w, cy, black_pixel);

	cpu_timer_stop(&sum_timer);

	if (!params->show_info) {
		GP_BackendFlip(backend);
		return NULL;
	}

	GP_Size th = GP_TextHeight(NULL);
	
	GP_Print(context, NULL, 11, 11, GP_ALIGN_RIGHT|GP_VALIGN_BOTTOM,
	         black_pixel, white_pixel, "%ux%u", w, h);
	
	GP_Print(context, NULL, 10, 10, GP_ALIGN_RIGHT|GP_VALIGN_BOTTOM,
	         white_pixel, black_pixel, "%ux%u", w, h);
	
	GP_Print(context, NULL, 11, 13 + th, GP_ALIGN_RIGHT|GP_VALIGN_BOTTOM,
	         black_pixel, white_pixel, "1:%3.3f", rat);
	
	GP_Print(context, NULL, 10, 12 + th, GP_ALIGN_RIGHT|GP_VALIGN_BOTTOM,
	         white_pixel, black_pixel, "1:%3.3f", rat);
	
	GP_Print(context, NULL, 11, 15 + 2 * th, GP_ALIGN_RIGHT|GP_VALIGN_BOTTOM,
	         black_pixel, white_pixel, "%s", img_name(params->img_path));
	
	GP_Print(context, NULL, 10, 14 + 2 * th, GP_ALIGN_RIGHT|GP_VALIGN_BOTTOM,
	         white_pixel, black_pixel, "%s", img_name(params->img_path));

	GP_BackendFlip(backend);

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
		GP_BackendExit(backend);
		exit(1);
	}
}

static void sighandler(int signo)
{
	if (backend != NULL)
		GP_BackendExit(backend);
	
	fprintf(stderr, "Got signal %i\n", signo);

	exit(1);
}

static void init_backend(const char *backend_opts)
{
	if (!strcmp(backend_opts, "fb")) {
		fprintf(stderr, "Initalizing framebuffer backend\n");
		backend = GP_BackendLinuxFBInit("/dev/fb0");
	}
	
	if (!strcmp(backend_opts, "SDL")) {
		fprintf(stderr, "Initalizing SDL backend\n");
		backend = GP_BackendSDLInit(800, 600, 0, 0);
	}

	if (!strcmp(backend_opts, "SDL:FS")) {
		fprintf(stderr, "Initalizing SDL fullscreen\n");
		backend = GP_BackendSDLInit(0, 0, 0, GP_SDL_FULLSCREEN);
	}

	if (backend == NULL) {
		fprintf(stderr, "Failed to initalize backend '%s'\n", backend_opts);
		exit(1);
	}
}

int main(int argc, char *argv[])
{
	GP_InputDriverLinux *drv = NULL;
	const char *input_dev = NULL;
	const char *backend_opts = "fb";
	int sleep_sec = -1;
	struct loader_params params = {NULL, 0, 0, 0};
	int opt;

	while ((opt = getopt(argc, argv, "b:Ii:Ps:r:")) != -1) {
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
		case 'b':
			backend_opts = optarg;
		break;
		default:
			fprintf(stderr, "Invalid paramter '%c'\n", opt);
		}
	}
	
	GP_SetDebugLevel(10);

	if (input_dev != NULL) {
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

	init_backend(backend_opts);

	context = backend->context;

	GP_EventSetScreenSize(context->w, context->h);
	
	black_pixel = GP_ColorToContextPixel(GP_COL_BLACK, context);
	white_pixel = GP_ColorToContextPixel(GP_COL_WHITE, context);

	GP_Fill(context, black_pixel);
	GP_BackendFlip(backend);

	int argf = optind;
	int argn = argf;
		
	params.show_progress_once = 1;
	params.img_path = argv[argf];
	show_image(&params);

	for (;;) {
		int ret;

		if (drv != NULL) {
			/* Initalize select */
			fd_set rfds;
			FD_ZERO(&rfds);
			FD_SET(drv->fd, &rfds);
			struct timeval tv = {.tv_sec = sleep_sec, .tv_usec = 0};
			struct timeval *tvp = sleep_sec != -1 ? &tv : NULL;
			
			ret = select(drv->fd + 1, &rfds, NULL, NULL, tvp);
		
			tv.tv_sec = sleep_sec;
	
			switch (ret) {
			case -1:
				GP_BackendExit(backend);
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
			if (sleep_sec != -1) {
				sleep(sleep_sec);
		
				argn++;
				if (argn >= argc)
					argn = argf;
			
				params.img_path = argv[argn];
				show_image(&params);
			}
		}

		if (backend->Poll)
			GP_BackendPoll(backend);

		usleep(1000);

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
					GP_BackendExit(backend);
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

	GP_BackendExit(backend);

	return 0;
}
