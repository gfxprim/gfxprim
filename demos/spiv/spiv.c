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
static int show_progress = 0;

struct loader_params {
	/* current image path */
	const char *img_path;
	/* current resize ratio */
	float rat;

	/* show loader progress */
	long show_progress:1;
	long show_progress_once:2;
	/* show image info in topleft corner */
	long show_info:3;
	/* use nearest neighbour resampling first */
	long show_nn_first:4;
	/* use dithering when blitting to display */
	long use_dithering:5;
	/* use low pass before resampling */
	long use_low_pass:6;
	/* image orientation 0, 90, 180, 270 */
	int rotate;
	/* resampling method */
	int resampling_method;

	/* caches for loaded images */
	struct image_cache *img_resized_cache;
	struct image_cache *img_orig_cache;
};

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

	img = image_cache_get(params->img_orig_cache, params->img_path, 0, 0, elevate);

	/* Image not cached, load it */
	if (img == NULL) {
		fprintf(stderr, "Loading '%s'\n", params->img_path);

		cpu_timer_start(&timer, "Loading");
		if ((img = GP_LoadImage(params->img_path, &callback)) == NULL) {
			
			if (errno == ECANCELED)
				return NULL;

			GP_Fill(context, black_pixel);
			GP_Print(context, NULL, context->w/2, context->h/2,
		        	 GP_ALIGN_CENTER|GP_VALIGN_CENTER, white_pixel, black_pixel,
				 "Failed to load image :( (%s)", strerror(errno));
			GP_BackendFlip(backend);
			return NULL;
		}
		
		/* Workaround */
		if (img->pixel_type != GP_PIXEL_RGB888) {
			GP_Context *tmp;
			tmp = GP_ContextConvertAlloc(img, GP_PIXEL_RGB888);
			GP_ContextFree(img);
			img = tmp;
		}

		image_cache_put(params->img_orig_cache, img, params->img_path, 0, 0);
		
		cpu_timer_stop(&timer);
	}

	return img; 
}

/*
 * Updates display.
 */
static void update_display(struct loader_params *params, GP_Context *img)
{
	GP_Context *context = backend->context;
	struct cpu_timer timer;
	GP_ProgressCallback callback = {.callback = image_loader_callback};

	switch (params->rotate) {
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
		return;

	uint32_t cx = (context->w - img->w)/2;
	uint32_t cy = (context->h - img->h)/2;

	GP_Context sub_display;

	cpu_timer_start(&timer, "Blitting");
	
	if (params->use_dithering) {
		callback.priv = "Dithering";
		GP_SubContext(context, &sub_display, cx, cy, img->w, img->h);
		GP_FilterFloydSteinberg_RGB888(img, &sub_display, NULL);
	//	GP_FilterHilbertPeano_RGB888(img, &sub_display, NULL);
	} else {
		GP_Blit_Raw(img, 0, 0, img->w, img->h, context, cx, cy);
	}
	
	cpu_timer_stop(&timer);
	
	if (params->rotate)
		GP_ContextFree(img);

	/* clean up the rest of the display */
	GP_FillRectXYWH(context, 0, 0, cx, context->h, black_pixel);
	GP_FillRectXYWH(context, 0, 0, context->w, cy, black_pixel);
	GP_FillRectXYWH(context, img->w + cx, 0, context->w - img->w - cx, context->h, black_pixel);
	GP_FillRectXYWH(context, 0, img->h + cy, context->w, context->h - img->h - cy, black_pixel);
	
	set_caption(params->img_path, params->rat); 

	if (!params->show_info) {
		GP_BackendFlip(backend);
		return;
	}

	GP_Size th = GP_TextHeight(NULL);
	
	GP_Print(context, NULL, 11, 11, GP_ALIGN_RIGHT|GP_VALIGN_BOTTOM,
	         black_pixel, white_pixel, "%ux%u", img->w, img->h);
	
	GP_Print(context, NULL, 10, 10, GP_ALIGN_RIGHT|GP_VALIGN_BOTTOM,
	         white_pixel, black_pixel, "%ux%u", img->w, img->h);
	
	GP_Print(context, NULL, 11, 13 + th, GP_ALIGN_RIGHT|GP_VALIGN_BOTTOM,
	         black_pixel, white_pixel, "1:%3.3f", params->rat);
	
	GP_Print(context, NULL, 10, 12 + th, GP_ALIGN_RIGHT|GP_VALIGN_BOTTOM,
	         white_pixel, black_pixel, "1:%3.3f", params->rat);
	
	GP_Print(context, NULL, 11, 15 + 2 * th, GP_ALIGN_RIGHT|GP_VALIGN_BOTTOM,
	         black_pixel, white_pixel, "%s", img_name(params->img_path));
	
	GP_Print(context, NULL, 10, 14 + 2 * th, GP_ALIGN_RIGHT|GP_VALIGN_BOTTOM,
	         white_pixel, black_pixel, "%s", img_name(params->img_path));
	
	GP_Print(context, NULL, 11, 17 + 3 * th, GP_ALIGN_RIGHT|GP_VALIGN_BOTTOM,
	         black_pixel, white_pixel, "%s%s",
		 params->use_low_pass && params->rat < 1 ? "Gaussian LP + " : "",
		 GP_InterpolationTypeName(params->resampling_method));
	
	GP_Print(context, NULL, 10, 16 + 3 * th, GP_ALIGN_RIGHT|GP_VALIGN_BOTTOM,
	         white_pixel, black_pixel, "%s%s", 
		 params->use_low_pass && params->rat < 1 ? "Gaussian LP + " : "",
		 GP_InterpolationTypeName(params->resampling_method));

	GP_BackendFlip(backend);
}

GP_Context *load_resized_image(struct loader_params *params, GP_Size w, GP_Size h)
{
	long cookie = (w & 0xffff) | (h & 0xffff)<<16;
	GP_Context *img, *res = NULL;
	struct cpu_timer timer;
	GP_ProgressCallback callback = {.callback = image_loader_callback};
	
	int key = (params->resampling_method<<1) | !!(params->use_low_pass);

	/* Try to get resized cached image */
	img = image_cache_get(params->img_resized_cache, params->img_path, cookie, key, 1);

	if (img != NULL)
		return img;
	
	/* Otherwise load image and resize it */
	if ((img = load_image(params, 1)) == NULL)
		return NULL;

	if (params->show_nn_first) {
		/* Do simple interpolation and blit the result */
		GP_Context *nn = GP_FilterResizeNNAlloc(img, w, h, NULL);
		if (nn != NULL) {
			update_display(params, nn);
			GP_ContextFree(nn);
		}
	}

	/* Do low pass filter */
	if (params->use_low_pass && params->rat < 1) {
		cpu_timer_start(&timer, "Blur");
		callback.priv = "Blurring Image";
		
		res = GP_FilterGaussianBlurAlloc(img, 0.3/params->rat,
		                                 0.3/params->rat, &callback);

		if (res == NULL)
			return NULL;
		
		img = res;
		
		cpu_timer_stop(&timer);
	}
	
//	img->gamma = GP_GammaAcquire(img->pixel_type, 2.2);

	cpu_timer_start(&timer, "Resampling");
	callback.priv = "Resampling Image";
	GP_Context *i1 = GP_FilterResize(img, NULL, params->resampling_method, w, h, &callback);
//	img->gamma = NULL;
//	GP_Context *i2 = GP_FilterResize(img, NULL, params->resampling_method, w, h, &callback);
//	img = GP_FilterDifferenceAlloc(i2, i1, NULL);
//	img = GP_FilterInvert(img, NULL, NULL);
	img = i1;
//	if (params->resampling_method == GP_INTERP_CUBIC_INT)
//		GP_FilterEdgeSharpening(img, img, 0.2, NULL);
	cpu_timer_stop(&timer);

	/* Free low passed context if needed */
	GP_ContextFree(res);

	if (img == NULL)
		return NULL;

	image_cache_put(params->img_resized_cache, img, params->img_path, cookie, key);
	
	return img;
}

static void *image_loader(void *ptr)
{
	struct loader_params *params = ptr;
	struct cpu_timer sum_timer;
	GP_Context *img, *context = backend->context;

	cpu_timer_start(&sum_timer, "sum");
	
	show_progress = params->show_progress || params->show_progress_once;
	params->show_progress_once = 0;

	/* Figure out rotation */
	GP_Size w, h;

	switch (params->rotate) {
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

	params->rat = calc_img_size(img->w, img->h, w, h);

	w = img->w;
	h = img->h;

	img = load_resized_image(params, w * params->rat + 0.5, h * params->rat + 0.5);

	if (img == NULL)
		return NULL;

	image_cache_print(params->img_resized_cache);
	image_cache_print(params->img_orig_cache);

	update_display(params, img);
	
	cpu_timer_stop(&sum_timer);
	
	return NULL;
}

static pthread_t loader_thread = (pthread_t)0;

static void stop_loader(void)
{
	if (loader_thread) {
		abort_flag = 1;
		pthread_join(loader_thread, NULL);
		loader_thread = (pthread_t)0;
		abort_flag = 0;
	}
}

static void show_image(struct loader_params *params, const char *path)
{
	int ret;

	if (path != NULL)
		params->img_path = path;

	/* stop previous loader thread */
	stop_loader();

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
		usleep(10000);
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

static void init_caches(struct loader_params *params)
{
	size_t size = image_cache_get_ram_size();
	unsigned int resized_size = (1024 * size)/10;
	unsigned int orig_size = (1024 * size)/50;

	if (resized_size > 100 * 1024 * 1024)
		resized_size = 100 * 1024 * 1024;

	if (orig_size > 40 * 1024 * 1024)
		orig_size = 40 * 1024 * 1024;

	GP_DEBUG(1, "Cache sizes original = %u, resized = %u",
	         orig_size, resized_size);

	params->img_resized_cache = image_cache_create(resized_size);
	params->img_orig_cache = image_cache_create(orig_size);

//	params->img_resized_cache = NULL;
//	params->img_orig_cache = NULL;
}

/*
 * Ask backend to resize window. Once window is resized we will
 * get SYS_RESIZE event, see the main event loop.
 */
static void resize_backend(struct loader_params *params, float ratio, int shift_flag)
{
	GP_Context *img = load_image(params, 1);

	if (!shift_flag)
		ratio = 1.00 / ratio;

	unsigned int w = img->w * ratio + 0.5;
	unsigned int h = img->h * ratio + 0.5;

	GP_BackendResize(backend, w, h);
}

static void print_help(void)
{
	printf("Usage: spiv [opts] images\n\n");
	printf("-I\n\tshow image info (filename and size)\n\n");
	printf("-P\n\tshow loading progress\n\n");
	printf("-f\n\tuse floyd-steinberg dithering\n\n");
	printf("-s sec\n\tsleep interval in seconds\n\n");
	printf("-c\n\tturns on bicubic resampling (experimental)\n\n");
	printf("-d level\n\tsets GFXprim debug level\n\n");
	printf("-e pixel_type\n\tturns on backend type emulation\n");
	printf("\tfor example -e G1 sets 1-bit grayscale\n\n");
	printf("-r angle\n\trotate display 90,180 or 270 degrees\n\n");
	printf("-b\n\tpass backend init string to backend init\n");
	printf("\tpass -b help for more info\n\n");

	printf("Keyboard control:\n\n");
	printf("I      - toggle show info\n");
	printf("P      - toggle show progress\n");
	printf("R      - rotate by 90 degrees\n");
	printf("]      - change to next resampling method\n");
	printf("[      - change to prev resampling method\n");
	printf("        (current method is shown in info\n");
	printf("L      - toggle low pass filter\n");
	printf("D      - drop image cache\n");
	printf("\nEsc\n");
	printf("Enter\n");
	printf("Q      - quit spiv\n\n");
	printf("PgDown - move ten image forward\n");
	printf("PgUp   - move ten image backward\n");
	printf("\nRight\n");
	printf("Up\n");
	printf("Space  - move to the next image\n");
	printf("\nLeft\n");
	printf("Down\n");
	printf("BckSpc - move to the prev image\n");
	printf("\n");
	printf("1      - resize spiv window to the image size\n");
	printf("2      - resize spiv window to the half of the image size\n");
	printf("3      - resize spiv window to the third of the image size\n");
	printf("...\n");
	printf("9      - resize spiv window to the ninth of the image size\n");
	printf("\n");
	printf("Shift 2 - resize spiv window twice of the image size\n");
	printf("Shift 3 - resize spiv window three times of the image size\n");
	printf("...\n\n");

	printf("Some cool options to try:\n\n");
	printf("spiv -e G1 -f images\n");
	printf("\truns spiv in 1-bit bitmap mode and turns on dithering\n\n");
	printf("spiv -b 'X11:ROOT_WIN' images\n");
	printf("\truns spiv using X root window as backend window\n\n");
	printf("spiv -b 'X11:CREATE_ROOT' images\n");
	printf("\tSame as abowe but works in KDE\n");

}

int main(int argc, char *argv[])
{
	GP_Context *context = NULL;
	const char *backend_opts = "X11";
	int sleep_sec = -1;
	int opt, debug_level = 0;
	int shift_flag;
	GP_PixelType emul_type = GP_PIXEL_UNKNOWN;
	
	struct loader_params params = {
		.img_path = NULL, 
		
		.show_progress = 0,
		.show_progress_once = 0,
		.show_info = 0,
		.show_nn_first = 0,
		.use_dithering = 0,
		.rotate = 0,
		.resampling_method = GP_INTERP_LINEAR_LF_INT,

		.img_resized_cache = NULL,
		.img_orig_cache = NULL,
	};

	while ((opt = getopt(argc, argv, "b:cd:e:fhIPs:r:")) != -1) {
		switch (opt) {
		case 'I':
			params.show_info = 1;
		break;
		case 'P':
			params.show_progress = 1;
		break;
		case 'f':
			params.use_dithering = 1;
		break;
		case 's':
			sleep_sec = atoi(optarg);
		break;
		case 'c':
			params.resampling_method = GP_INTERP_CUBIC_INT;
			/* Cubic resampling needs low pass */
			params.use_low_pass = 1;
			/* Cubic resampling is slow, show nn first */
			params.show_nn_first = 1;
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
				params.rotate = 90;
			else if (!strcmp(optarg, "180"))
				params.rotate = 180;
			else if (!strcmp(optarg, "270"))
				params.rotate = 270;
		case 'b':
			backend_opts = optarg;
		break;
		case 'h':
			print_help();
			exit(0);
		break;
		default:
			fprintf(stderr, "Invalid paramter '%c'\n", opt);
			print_help();
			return 1;
		}
	}
	
	GP_SetDebugLevel(debug_level);

	signal(SIGINT, sighandler);
	signal(SIGSEGV, sighandler);
	signal(SIGBUS, sighandler);
	signal(SIGABRT, sighandler);
	
	init_caches(&params);

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
		
			shift_flag = GP_EventGetKey(&ev, GP_KEY_LEFT_SHIFT) ||
			             GP_EventGetKey(&ev, GP_KEY_RIGHT_SHIFT);

			switch (ev.type) {
			case GP_EV_REL:
				switch (ev.code) {
				case GP_EV_REL_WHEEL:
					if (ev.val.val > 0)
						goto next;

					if (ev.val.val < 0)
						goto prev;
				break;
				}
			break;
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
					params.rotate += 90;
					if (params.rotate > 270)
						params.rotate = 0;
					
					params.show_progress_once = 1;
					show_image(&params, NULL);
				break;
				case GP_KEY_RIGHT_BRACE:
					params.resampling_method++;

					if (params.resampling_method > GP_INTERP_MAX)
						params.resampling_method = 0;
				
					if (params.resampling_method == GP_INTERP_LINEAR_LF_INT) {
						params.use_low_pass = 0;
						params.show_nn_first = 0;
					} else {
						params.use_low_pass = 1;
						params.show_nn_first = 1;
					}

					params.show_progress_once = 1;
					show_image(&params, argv[argn]);
				break;
				case GP_KEY_LEFT_BRACE:
					if (params.resampling_method == 0)
						params.resampling_method = GP_INTERP_MAX;
					else
						params.resampling_method--;
					
					if (params.resampling_method == GP_INTERP_LINEAR_LF_INT) {
						params.use_low_pass = 0;
						params.show_nn_first = 0;
					} else {
						params.use_low_pass = 1;
						params.show_nn_first = 1;
					}
					
					params.show_progress_once = 1;
					show_image(&params, argv[argn]);
				break;
				case GP_KEY_L:
					params.use_low_pass = !params.use_low_pass;
					
					params.show_progress_once = 1;
					show_image(&params, argv[argn]);
				break;
				case GP_KEY_D:
					image_cache_drop(params.img_resized_cache);
					image_cache_drop(params.img_orig_cache);
				break;
				case GP_KEY_ESC:
				case GP_KEY_ENTER:
				case GP_KEY_Q:
					image_cache_drop(params.img_resized_cache);
					image_cache_drop(params.img_orig_cache);
					GP_BackendExit(backend);
					return 0;
				break;
				case GP_KEY_PAGE_UP:
					argn+=10;
					//TODO
					if (argn >= argc)
						argn = argf; 
					
					params.show_progress_once = 1;
					show_image(&params, argv[argn]);
				break;
				case GP_KEY_PAGE_DOWN:
					argn-=10;
					//TODO
					if (argn < argf)
						argn = argc - 1 ; 
					
					params.show_progress_once = 1;
					show_image(&params, argv[argn]);
				break;
				next:
				case GP_KEY_RIGHT:
				case GP_KEY_UP:
				case GP_KEY_SPACE:
					argn++;
					if (argn >= argc)
						argn = argf;
					
					params.show_progress_once = 1;
					show_image(&params, argv[argn]);
				break;
				prev:
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
					resize_backend(&params, 1, shift_flag);
				break;
				case GP_KEY_2:
					resize_backend(&params, 2, shift_flag);
				break;
				case GP_KEY_3:
					resize_backend(&params, 3, shift_flag);
				break;
				case GP_KEY_4:
					resize_backend(&params, 4, shift_flag);
				break;
				case GP_KEY_5:
					resize_backend(&params, 5, shift_flag);
				break;
				case GP_KEY_6:
					resize_backend(&params, 6, shift_flag);
				break;
				case GP_KEY_7:
					resize_backend(&params, 7, shift_flag);
				break;
				case GP_KEY_8:
					resize_backend(&params, 8, shift_flag);
				break;
				case GP_KEY_9:
					resize_backend(&params, 9, shift_flag);
				break;
				}
			break;
			case GP_EV_SYS:
				switch (ev.code) {
				case GP_EV_SYS_RESIZE:
					/* stop loader thread before resizing backend buffer */
					stop_loader();
					GP_BackendResizeAck(backend);
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
