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

   SPIV -- Simple but Powerfull Image Viewer.

  */

#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <string.h>
#include <pthread.h>

#include <GP.h>
#include <backends/GP_Backends.h>
#include <input/GP_InputDriverLinux.h>

#include "image_cache.h"
#include "cpu_timer.h"

static GP_Pixel black_pixel;
static GP_Pixel white_pixel;

static GP_Backend *backend = NULL;

/* image loader thread */
static int abort_flag = 0;
static int rotate = 0;
static int show_progress = 0;
static int resampling_method = GP_INTERP_LINEAR_LF_INT;
static int dithering = 0;

static int image_loader_callback(GP_ProgressCallback *self)
{
	static GP_Size size = 0;
	GP_Context *c = backend->context;

	if (abort_flag)
		return 1;

	if (!show_progress)
		return 0;

	char buf[100];

	snprintf(buf, sizeof(buf), "%s ... %-3.1f%%",
	         (const char*)self->priv, self->percentage);

	int align = GP_ALIGN_CENTER|GP_VALIGN_ABOVE;

	GP_TextClear(c, NULL, c->w/2, c->h - 4, align,
	             black_pixel, GP_MAX(size, GP_TextWidth(NULL, buf)));

	GP_Text(c, NULL, c->w/2, c->h - 4, align,
	        white_pixel, black_pixel, buf);

	size = GP_TextWidth(NULL, buf);

	GP_BackendUpdateRect(backend, c->w/2 - size/2 - 1, c->h - 4,
	                     c->w/2 + size/2 + 1, c->h - 4 - GP_TextHeight(NULL));

	return 0;
}

struct loader_params {
	const char *img_path;
	int show_progress;
	int show_progress_once;
	int show_info;

	/* cached loaded images */
	struct image_cache *image_cache;
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

	return img_path;
}

static void set_caption(const char *path, float rat)
{
	char buf[256];

	snprintf(buf, sizeof(buf), "Spiv ~ %s 1:%3.3f", img_name(path), rat);

	GP_BackendSetCaption(backend, buf);
}

/*
 * Loads image
 */
GP_Context *load_image(struct loader_params *params, int elevate)
{
	struct cpu_timer timer;
	GP_Context *img, *context = backend->context;
	
	GP_ProgressCallback callback = {.callback = image_loader_callback,
	                                .priv = "Loading image"};

	img = image_cache_get(params->image_cache, params->img_path, 0, 0, elevate);

	/* Image not cached, load it */
	if (img == NULL) {
		show_progress = params->show_progress || params->show_progress_once;
		params->show_progress_once = 0;

		fprintf(stderr, "Loading '%s'\n", params->img_path);

		cpu_timer_start(&timer, "Loading");
		if ((img = GP_LoadImage(params->img_path, &callback)) == NULL) {
			GP_Fill(context, black_pixel);
			GP_Print(context, NULL, context->w/2, context->h/2,
		        	 GP_ALIGN_CENTER|GP_VALIGN_CENTER, white_pixel, black_pixel,
				 "Failed to load image :( (%s)", strerror(errno));
			GP_BackendFlip(backend);
			return NULL;
		}
		
		/* Workaround */
		if (img->pixel_type != GP_PIXEL_RGB888) {
			GP_Context *tmp = GP_ContextConvert(img, GP_PIXEL_RGB888);
			GP_ContextFree(img);
			img = tmp;
		}
	
		image_cache_put(params->image_cache, img, params->img_path, 0, 0);
		
		cpu_timer_stop(&timer);
	}

	return img; 
}

GP_Context *load_resized_image(struct loader_params *params, GP_Size w, GP_Size h, float rat)
{
	long cookie = (w & 0xffff) | (h & 0xffff)<<16;
	GP_Context *img, *res = NULL;
	struct cpu_timer timer;
	GP_ProgressCallback callback = {.callback = image_loader_callback};

	/* Try to get resized cached image */
	img = image_cache_get(params->image_cache, params->img_path, cookie, resampling_method, 1);

	if (img != NULL)
		return img;
	
	/* Otherwise load image and resize it */
	if ((img = load_image(params, 1)) == NULL)
		return NULL;

	/* Do low pass filter */
	if (resampling_method != GP_INTERP_LINEAR_LF_INT && rat < 1) {
		cpu_timer_start(&timer, "Blur");
		callback.priv = "Blurring Image";
		
		res = GP_FilterGaussianBlur(img, NULL, 0.4/rat, 0.4/rat, &callback);
		
		if (res == NULL)
			return NULL;
		
		img = res;

		cpu_timer_stop(&timer);
	}

	cpu_timer_start(&timer, "Resampling");
	callback.priv = "Resampling Image";
	img = GP_FilterResize(img, NULL, resampling_method, w, h, &callback);
	cpu_timer_stop(&timer);

	/* Free low passed context if needed */
	GP_ContextFree(res);

	if (img == NULL)
		return NULL;

	image_cache_put(params->image_cache, img, params->img_path, cookie, resampling_method);
	
	return img;
}

/*
 * This function tries to resize spiv window
 * and if succedes blits the image directly to the screen.
 */
static int resize_backend_and_blit(struct loader_params *params)
{
	GP_Context *img = load_image(params, 1);
	
	if (GP_BackendResize(backend, img->w, img->h))
		return 1;

	GP_Blit_Raw(img, 0, 0, img->w, img->h, backend->context, 0, 0);
	GP_BackendFlip(backend);
	set_caption(params->img_path, 1); 

	return 0;
}

static void *image_loader(void *ptr)
{
	struct loader_params *params = ptr;
	struct cpu_timer timer, sum_timer;
	GP_Context *img, *context = backend->context;
	GP_ProgressCallback callback = {.callback = image_loader_callback};

	cpu_timer_start(&sum_timer, "sum");

	/* Figure out rotation */
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

	if ((img = load_image(params, 0)) == NULL)
		return NULL;

	float rat = calc_img_size(img->w, img->h, w, h);

	w = img->w;
	h = img->h;

	img = load_resized_image(params, img->w * rat + 0.5, img->h * rat + 0.5, rat);

	if (img == NULL)
		return NULL;

	image_cache_print(params->image_cache);

	switch (rotate) {
	case 0:
	break;
	case 90:
		callback.priv = "Rotating image (90)";
		img = GP_FilterRotate90_Alloc(img, &callback);
	break;
	case 180:
		callback.priv = "Rotating image (180)";
		img = GP_FilterRotate180_Alloc(img, &callback);
	break;
	case 270:
		callback.priv = "Rotating image (270)";
		img = GP_FilterRotate270_Alloc(img, &callback);
	break;
	}
	
	if (img == NULL)
		return NULL;

	uint32_t cx = (context->w - img->w)/2;
	uint32_t cy = (context->h - img->h)/2;

	GP_Context sub_display;

	cpu_timer_start(&timer, "Blitting");
	
	if (dithering) {
		callback.priv = "Dithering";
		GP_ContextSubContext(context, &sub_display, cx, cy, img->w, img->h);
	//	GP_FilterFloydSteinberg_RGB888(ret, &sub_display, NULL);
		GP_FilterHilbertPeano_RGB888(img, &sub_display, NULL);
	} else {
		GP_Blit_Raw(img, 0, 0, img->w, img->h, context, cx, cy);
	}
	
	cpu_timer_stop(&timer);
	
	if (rotate)
		GP_ContextFree(img);

	/* clean up the rest of the display */
	GP_FillRectXYWH(context, 0, 0, cx, context->h, black_pixel);
	GP_FillRectXYWH(context, 0, 0, context->w, cy, black_pixel);
	GP_FillRectXYWH(context, img->w + cx, 0, context->w - img->w - cx, context->h, black_pixel);
	GP_FillRectXYWH(context, 0, img->h + cy, context->w, context->h - img->h - cy, black_pixel);

	cpu_timer_stop(&sum_timer);

	set_caption(params->img_path, rat); 

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

static void show_image(struct loader_params *params, const char *path)
{
	int ret;

	/* stop previous loader thread */
	if (loader_thread) {
		abort_flag = 1;
		pthread_join(loader_thread, NULL);
		loader_thread = (pthread_t)0;
		abort_flag = 0;
	}
	
	if (path != NULL)
		params->img_path = path;

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
	backend = GP_BackendInit(backend_opts, "Spiv", stderr); 
	
	if (backend == NULL) {
		fprintf(stderr, "Failed to initalize backend '%s'\n", backend_opts);
		exit(1);
	}
}

static int alarm_fired = 0;

static void alarm_handler(int signo)
{
	alarm_fired = 1;
}

static int wait_for_event(int sleep_msec)
{
	static int sleep_msec_count = 0;
	static int alarm_set = 0;

	if (sleep_msec < 0) {
		GP_BackendPoll(backend);
		return 0;
	}

	/* We can't sleep on backend fd, because the backend doesn't export it. */
	if (backend->fd < 0) {
		GP_BackendPoll(backend);
		usleep(10000);

		sleep_msec_count += 10;

		if (sleep_msec_count >= sleep_msec) {
			sleep_msec_count = 0;
			return 1;
		}

		return 0;
	}

	/* Initalize select */
	fd_set rfds;
	FD_ZERO(&rfds);
	
	FD_SET(backend->fd, &rfds);

	if (!alarm_set) {
		signal(SIGALRM, alarm_handler);
		alarm(sleep_msec / 1000);
		alarm_fired = 0;
		alarm_set = 1;
	}

	struct timeval tv = {.tv_sec = sleep_msec / 1000,
	                     .tv_usec = (sleep_msec % 1000) * 1000};
	
	int ret = select(backend->fd + 1, &rfds, NULL, NULL, &tv);
	
	switch (ret) {
	case -1:
		if (errno == EINTR)
			return 1;
		
		GP_BackendExit(backend);
		exit(1);
	break;
	case 0:
		if (alarm_fired) {
			alarm_set = 0;
			return 1;
		}

		return 0;
	break;
	default:
		GP_BackendPoll(backend);
		return 0;
	}
}

int main(int argc, char *argv[])
{
	GP_Context *context = NULL;
	const char *backend_opts = "X11";
	int sleep_sec = -1;
	struct loader_params params = {NULL, 0, 0, 0, NULL};
	int opt, debug_level = 0;
	GP_PixelType emul_type = GP_PIXEL_UNKNOWN;

	params.image_cache = image_cache_create(0);

	while ((opt = getopt(argc, argv, "b:cd:e:fIPs:r:")) != -1) {
		switch (opt) {
		case 'I':
			params.show_info = 1;
		break;
		case 'P':
			params.show_progress = 1;
		break;
		case 'f':
			dithering = 1;
		break;
		case 's':
			sleep_sec = atoi(optarg);
		break;
		case 'c':
			resampling_method = GP_INTERP_CUBIC_INT;
		break;
		case 'd':
			debug_level = atoi(optarg);
		break;
		case 'e':
			emul_type = GP_PixelTypeByName(optarg);

			if (emul_type == GP_PIXEL_UNKNOWN) {
				fprintf(stderr, "Invalid pixel type '%s'\n", optarg);
				return 1;
			}
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
	
	GP_SetDebugLevel(debug_level);

	signal(SIGINT, sighandler);
	signal(SIGSEGV, sighandler);
	signal(SIGBUS, sighandler);
	signal(SIGABRT, sighandler);

	init_backend(backend_opts);

	if (emul_type != GP_PIXEL_UNKNOWN)
		backend = GP_BackendVirtualInit(backend, emul_type, GP_BACKEND_CALL_EXIT);

	context = backend->context;

	GP_EventSetScreenSize(context->w, context->h);
	
	black_pixel = GP_ColorToContextPixel(GP_COL_BLACK, context);
	white_pixel = GP_ColorToContextPixel(GP_COL_WHITE, context);

	GP_Fill(context, black_pixel);
	GP_BackendFlip(backend);

	int argf = optind;
	int argn = argf;

	params.show_progress_once = 1;
	show_image(&params, argv[argf]);
		
	for (;;) {
		/* wait for event or a timeout */
		if (wait_for_event(sleep_sec * 1000)) {
			argn++;
			if (argn >= argc)
				argn = argf;
					
			show_image(&params, argv[argn]);
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
					show_image(&params, NULL);
				break;
				case GP_KEY_P:
					params.show_progress = !params.show_progress;
				break;
				case GP_KEY_R:
					rotate += 90;
					if (rotate > 270)
						rotate = 0;
					
					params.show_progress_once = 1;
					show_image(&params, NULL);
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
					show_image(&params, argv[argn]);
				break;
				case GP_KEY_BACKSPACE:
				case GP_KEY_LEFT:
				case GP_KEY_DOWN:
					argn--;

					if (argn < argf)
						argn = argc - 1;

					params.show_progress_once = 1;
					show_image(&params, argv[argn]);
				break;
				case GP_KEY_1:
					resize_backend_and_blit(&params);
				break;
				}
			break;
			case GP_EV_SYS:
				switch (ev.code) {
				case GP_EV_SYS_RESIZE:
					GP_BackendResize(backend, ev.val.sys.w, ev.val.sys.h);
					GP_Fill(backend->context, 0);
					params.show_progress_once = 1;
					show_image(&params, NULL);
				break;
				case GP_EV_SYS_QUIT:
					GP_BackendExit(backend);
					return 0;
				break;
				}
			break;
			}
		}
	}

	GP_BackendExit(backend);

	return 0;
}
