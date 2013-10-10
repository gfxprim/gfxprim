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

   SPIV -- Simple yet Powerful Image Viewer.

  */

#include <errno.h>
#include <signal.h>
#include <string.h>
#include <pthread.h>

#include <GP.h>

#include "image_cache.h"
#include "image_list.h"
#include "image_loader.h"
#include "image_actions.h"
#include "spiv_help.h"
#include "spiv_config.h"
#include "cpu_timer.h"

static GP_Pixel black_pixel;
static GP_Pixel white_pixel;
static GP_Pixel gray_pixel;

static GP_Backend *backend = NULL;

/* image loader thread */
static int abort_flag = 0;
static int show_progress = 0;
static int loader_running = 0;

enum zoom_type {
	/*
	 * Resize image to fit current size of the window.
	 */
	ZOOM_FIT,

	/*
	 * Use zoom set in zoom float and zoom offsets.
	 */
	ZOOM_FIXED,

	/*
	 * Fixed zoom but spiv tries to change
	 * the window size to fit the image size
	 */
	ZOOM_FIXED_WIN,

	/*
	 * Do not upscale images but downscale them
	 * if they are too big.
	 */
	ZOOM_FIT_DOWNSCALE,
};

struct loader_params {
	/* current resize ratio */
	float rat;

	long show_progress_once:2;
	/* use nearest neighbour resampling first */
	long show_nn_first:4;
	/* use low pass before resampling */
	long use_low_pass:6;
	/* resampling method */
	int resampling_method;

	/* slideshow sleep */
	int sleep_ms;

	/* offset in pixels */
	unsigned int zoom_x_offset;
	unsigned int zoom_y_offset;

	/* zoom */
	enum zoom_type zoom_type;
	float zoom;

	/* caches for loaded images */
	struct image_cache *img_resized_cache;
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

	size = GP_TextWidth(NULL, buf);

	int start = c->w/2 - size/2 - 10;
	int end   = c->w/2 + size/2 + 10;
	int middle = start + (end - start) * self->percentage / 100;
	int top = c->h - GP_TextHeight(NULL) - 11;

	GP_FillRectXYXY(c, start, c->h - 1, middle, top, gray_pixel);
	GP_FillRectXYXY(c, middle, c->h - 1, end, top, black_pixel);

	GP_Text(c, NULL, c->w/2, c->h - 5, align,
	        white_pixel, black_pixel, buf);

	GP_BackendUpdateRect(backend, start, c->h - 1, end, top);

	return 0;
}

static GP_Context *load_image(int elevate);

/*
 * Ask backend to resize window may not be implemented or authorized. If
 * backend (window) is resized we will get SYS_RESIZE event, see the main event
 * loop.
 */
static void resize_backend(float ratio, int shift_flag)
{
	GP_Context *img = load_image(1);

	if (!shift_flag)
		ratio = 1.00 / ratio;

	unsigned int w = img->w * ratio + 0.5;
	unsigned int h = img->h * ratio + 0.5;

	GP_BackendResize(backend, w, h);
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
static GP_Context *load_image(int elevate)
{
	GP_Context *img;
	GP_ProgressCallback callback = {.callback = image_loader_callback,
	                                .priv = "Loading image"};

	img = image_loader_get_image(&callback, elevate);

	if (img)
		return img;

	GP_Context *ctx = backend->context;

	GP_Fill(ctx, black_pixel);
	GP_Print(ctx, NULL, ctx->w/2, ctx->h/2 - 10,
	         GP_ALIGN_CENTER|GP_VALIGN_CENTER, white_pixel, black_pixel,
	         "'%s'", image_loader_img_path());
	GP_Print(ctx, NULL, ctx->w/2, ctx->h/2 + 10,
	         GP_ALIGN_CENTER|GP_VALIGN_CENTER, white_pixel, black_pixel,
	         "Failed to load image :( (%s)", strerror(errno));
	GP_BackendFlip(backend);

	return NULL;
}

/*
 * Fill context with chessboard-like pattern.
 */
static void pattern_fill(GP_Context *ctx, unsigned int x0, unsigned int y0,
                         unsigned int w, unsigned int h)
{
	unsigned int x, y;

	GP_Pixel g1 = GP_RGBToContextPixel(0x64, 0x64, 0x64, ctx);
	GP_Pixel g2 = GP_RGBToContextPixel(0x80, 0x80, 0x80, ctx);

	unsigned int wm = w/10 < 10 ? 10 : w/10;
	unsigned int hm = h/10 < 10 ? 10 : h/10;
	unsigned int wt = w/20 < 5  ?  5 : w/20;
	unsigned int ht = h/20 < 5  ?  5 : h/20;

	for (y = 0; y < h; y++) {
		for (x = 0; x < w; x++) {
			GP_Pixel pix;

			if ((x % wm < wt) ^ (y % hm < ht))
				pix = g1;
			else
				pix = g2;

			GP_PutPixel(ctx, x0 + x, y0 + y, pix);
		}
	}
}


static void info_printf(GP_Context *ctx, GP_Coord x, GP_Coord y,
                        const char *fmt, ...)
                        __attribute__ ((format (printf, 4, 5)));

static void info_printf(GP_Context *ctx, GP_Coord x, GP_Coord y,
                        const char *fmt, ...)
{
	va_list va, vac;

	va_start(va, fmt);

	va_copy(vac, va);
	GP_VPrint(ctx, NULL, x-1, y-1, GP_ALIGN_RIGHT|GP_VALIGN_BOTTOM,
	          black_pixel, white_pixel, fmt, vac);
	va_end(vac);

	GP_VPrint(ctx, NULL, x, y, GP_ALIGN_RIGHT|GP_VALIGN_BOTTOM,
	          white_pixel, black_pixel, fmt, va);

	va_end(va);
}

static void show_info(struct loader_params *params, GP_Context *img,
                      GP_Context *orig_img)
{
	GP_Context *context = backend->context;
	const char *img_path = image_loader_img_path();

	set_caption(img_path, params->rat);

	if (!config.show_info)
		return;

	GP_Size th = GP_TextHeight(NULL);

	info_printf(context, 10, 10, "%ux%u (%ux%u) 1:%3.3f",
	         img->w, img->h, orig_img->w, orig_img->h, params->rat);

	info_printf(context, 10, 12 + th, "%s", img_name(img_path));

	info_printf(context, 10, 14 + 2 * th, "%s%s",
		 params->use_low_pass && params->rat < 1 ? "Gaussian LP + " : "",
		 GP_InterpolationTypeName(params->resampling_method));

	unsigned int count = image_loader_count();
	unsigned int pos = image_loader_pos() + 1;

	info_printf(context, 10, 16 + 3 * th, "%u of %u", pos, count);

	if (!image_loader_is_in_dir())
		return;

	unsigned int dir_count = image_loader_dir_count();
	unsigned int dir_pos = image_loader_dir_pos() + 1;

	info_printf(context, 10, 18 + 4 * th,
		    "%u of %u in directory", dir_pos, dir_count);
}

static void update_display(struct loader_params *params, GP_Context *img,
                           GP_Context *orig_img)
{
	GP_Context *context = backend->context;
	struct cpu_timer timer;
	GP_ProgressCallback callback = {.callback = image_loader_callback};

	switch (config.orientation) {
	case ROTATE_0:
	break;
	case ROTATE_90:
		callback.priv = "Rotating image (90)";
		img = GP_FilterRotate90Alloc(img, &callback);
	break;
	case ROTATE_180:
		callback.priv = "Rotating image (180)";
		img = GP_FilterRotate180Alloc(img, &callback);
	break;
	case ROTATE_270:
		callback.priv = "Rotating image (270)";
		img = GP_FilterRotate270Alloc(img, &callback);
	break;
	}

	if (img == NULL)
		return;

	int cx = 0;
	int cy = 0;

	switch (params->zoom_type) {
	case ZOOM_FIT_DOWNSCALE:
	case ZOOM_FIT:
		cx = (context->w - img->w)/2;
		cy = (context->h - img->h)/2;
	break;
	case ZOOM_FIXED:
	case ZOOM_FIXED_WIN:
		cx = params->zoom_x_offset;
		cy = params->zoom_y_offset;
	break;
	}

	GP_Context sub_display;

	cpu_timer_start(&timer, "Blitting");

	if (config.floyd_steinberg) {
		callback.priv = "Dithering";
		GP_SubContext(context, &sub_display, cx, cy, img->w, img->h);
		GP_FilterFloydSteinberg(img, &sub_display, NULL);
	//	GP_FilterHilbertPeano(img, &sub_display, NULL);
	} else {
		if (GP_PixelHasFlags(img->pixel_type, GP_PIXEL_HAS_ALPHA))
			pattern_fill(context, cx, cy, img->w, img->h);

		GP_Blit_Clipped(img, 0, 0, img->w, img->h, context, cx, cy);
	}

	cpu_timer_stop(&timer);

	/* clean up the rest of the display */
	GP_FillRectXYWH(context, 0, 0, cx, context->h, black_pixel);
	GP_FillRectXYWH(context, 0, 0, context->w, cy, black_pixel);

	int w = context->w - img->w - cx;

	if (w > 0)
		GP_FillRectXYWH(context, img->w + cx, 0, w, context->h, black_pixel);

	int h = context->h - img->h - cy;

	if (h > 0)
		GP_FillRectXYWH(context, 0, img->h + cy, context->w, h, black_pixel);

	show_info(params, img, orig_img);

	if (config.orientation)
		GP_ContextFree(img);

	GP_BackendFlip(backend);
}

GP_Context *load_resized_image(struct loader_params *params, GP_Size w, GP_Size h)
{
	GP_Context *img, *res = NULL;
	struct cpu_timer timer;
	GP_ProgressCallback callback = {.callback = image_loader_callback};

	const char *img_path = image_loader_img_path();

	/* Try to get resized cached image */
	img = image_cache_get2(params->img_resized_cache, 1, "%s %ux%u r%i l%i",
	                       img_path, w, h, params->resampling_method,
	                       params->use_low_pass);

	if (img != NULL)
		return img;

	/* Otherwise load image and resize it */
	if ((img = load_image(1)) == NULL)
		return NULL;

	if (params->show_nn_first) {
		/* Do simple interpolation and blit the result */
		GP_Context *nn = GP_FilterResizeNNAlloc(img, w, h, NULL);
		if (nn != NULL) {
			update_display(params, nn, img);
			GP_ContextFree(nn);
		}
	}

	/* Do low pass filter */
	if (params->use_low_pass && params->rat < 1) {
		cpu_timer_start(&timer, "Blur");
		callback.priv = "Blurring Image";

		res = GP_FilterGaussianBlurAlloc(img, 0.4/params->rat,
		                                 0.4/params->rat, &callback);

		if (res == NULL)
			return NULL;

		img = res;

		cpu_timer_stop(&timer);
	}

//	img->gamma = GP_GammaAcquire(img->pixel_type, 0.45);

	cpu_timer_start(&timer, "Resampling");
	callback.priv = "Resampling Image";
	GP_Context *i1 = GP_FilterResizeAlloc(img, w, h, params->resampling_method, &callback);
	img = i1;
	cpu_timer_stop(&timer);

/*
	if (params->rat > 1.5) {
		cpu_timer_start(&timer, "Sharpening");
		callback.priv = "Sharpening";
		GP_FilterEdgeSharpening(i1, i1, 0.1, &callback);
		cpu_timer_stop(&timer);
	}
*/

	/* Free low passed context if needed */
	GP_ContextFree(res);

	if (img == NULL)
		return NULL;

	image_cache_put2(params->img_resized_cache, img, "%s %ux%u r%i l%i",
	                 img_path, w, h, params->resampling_method,
	                 params->use_low_pass);

	return img;
}

static float calc_img_size(struct loader_params *params,
                           uint32_t img_w, uint32_t img_h,
                           uint32_t src_w, uint32_t src_h)
{
	float w_rat;
	float h_rat;

	switch (config.orientation) {
	case ROTATE_0:
	case ROTATE_180:
	default:
	break;
	case ROTATE_90:
	case ROTATE_270:
		GP_SWAP(src_w, src_h);
	break;
	}

	switch (params->zoom_type) {
	case ZOOM_FIT_DOWNSCALE:
		if (img_w <= src_w && img_h <= src_h)
			return 1.00;
	case ZOOM_FIT:
		w_rat = 1.00 * src_w / img_w;
		h_rat = 1.00 * src_h / img_h;
		return GP_MIN(w_rat, h_rat);
	case ZOOM_FIXED:
		return params->zoom;
	case ZOOM_FIXED_WIN:
		resize_backend(params->zoom, 0);
		return params->zoom;
	}

	return 1.00;
}

static void *image_loader(void *ptr)
{
	struct loader_params *params = ptr;
	struct cpu_timer sum_timer;
	GP_Context *img, *orig_img, *context = backend->context;

	cpu_timer_start(&sum_timer, "sum");

	show_progress = config.show_progress || params->show_progress_once;
	params->show_progress_once = 0;

	if ((orig_img = load_image(0)) == NULL) {
		loader_running = 0;
		return NULL;
	}

	/* Figure out rotation */
	GP_Size w, h;

	params->rat = calc_img_size(params, orig_img->w, orig_img->h,
	                            context->w, context->h);

	w = orig_img->w * params->rat + 0.5;
	h = orig_img->h * params->rat + 0.5;

	/* Special case => no need to resize */
	if (w == orig_img->w && h == orig_img->h) {
		img = orig_img;
		goto update;
	}

	img = load_resized_image(params, w, h);

	if (img == NULL) {
		loader_running = 0;
		return NULL;
	}

update:
	update_display(params, img, orig_img);
	cpu_timer_stop(&sum_timer);

	loader_running = 0;

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

static void show_image(struct loader_params *params)
{
	int ret;

	/* stop previous loader thread */
	stop_loader();

	loader_running = 1;

	ret = pthread_create(&loader_thread, NULL, image_loader, (void*)params);

	if (ret) {
		fprintf(stderr, "Failed to start thread: %s\n", strerror(ret));
		GP_BackendExit(backend);
		exit(1);
	}
}

static void image_seek(struct loader_params *params,
                       enum img_seek_offset offset, int whence)
{
	/*
	 * We need to stop loader first because image loader seek may free
	 * image we are currently resamling.
	 */
	stop_loader();
	image_loader_seek(offset, whence);
	show_image(params);
}

static void set_zoom_offset(struct loader_params *params, int dx, int dy)
{
	params->zoom_x_offset += dx;
	params->zoom_y_offset += dy;
	show_image(params);
}

static void zoom_mul(struct loader_params *params, float mul)
{
	params->zoom *= mul;
	show_image(params);
}

static void sighandler(int signo)
{
	if (backend != NULL)
		GP_BackendExit(backend);

	fprintf(stderr, "Got signal %i\n", signo);

	_exit(1);
}

static void init_backend(const char *backend_opts)
{
	backend = GP_BackendInit(backend_opts, "Spiv", stderr);

	if (backend == NULL) {
		fprintf(stderr, "Failed to initalize backend '%s'\n", backend_opts);
		exit(1);
	}
}

/*
 * Figure out cache size depending on the size of RAM.
 *
 * Initialize cache, image loader.
 */
static int init_loader(struct loader_params *params, const char **argv)
{
	size_t size = image_cache_get_ram_size();
	unsigned int resized_size = (1024 * size)/10;
	unsigned int orig_size = (1024 * size)/50;

	if (resized_size > 100 * 1024 * 1024)
		resized_size = 100 * 1024 * 1024;

	GP_DEBUG(1, "Resized cache size = %u", resized_size);

	if (image_loader_init(argv, orig_size))
		return 1;

	params->img_resized_cache = image_cache_create(resized_size);

	return 0;
}

static uint32_t timer_callback(GP_Timer *self)
{
	struct loader_params *params = self->priv;
	static int retries = 0;

	/*
	 * If loader is still running, reschedule after 20ms
	 *
	 * If more than two seconds has passed, try load next
	 */
	if (loader_running && retries < 100) {
		printf("Loader still running %ims\n", 20 * retries);
		retries++;
		return 20;
	} else {
		retries = 0;
	}

	/*
	 * We need to stop loader first because image loader seek may free
	 * image we are currently resamling.
	 */
	stop_loader();
	image_loader_seek(IMG_CUR, 1);
	show_image(params);

	return params->sleep_ms;
}

int main(int argc, char *argv[])
{
	GP_Context *context = NULL;
	int shift_flag;
	int opts;

	struct loader_params params = {
		.show_progress_once = 0,
		.show_nn_first = 0,
		.resampling_method = GP_INTERP_LINEAR_LF_INT,

                .zoom_type = ZOOM_FIT,
                .zoom = 1,

		.img_resized_cache = NULL,

		.sleep_ms = 0,
	};

	GP_TIMER_DECLARE(timer, 0, 0, "Slideshow", timer_callback, &params);

	if (access("/etc/spiv.conf", R_OK) == 0)
		spiv_config_load("/etc/spiv.conf");

	if (getenv("HOME")) {
		char buf[256];

		snprintf(buf, sizeof(buf), "%s/%s", getenv("HOME"), ".spiv");

		if (access(buf, R_OK) == 0)
			spiv_config_load(buf);
	}

	opts = spiv_config_parse_args(argc, argv);

	if (opts < 0) {
		print_help();
		return 1;
	}

	cpu_timer_switch(config.timers);
	params.sleep_ms = 1000 * config.slideshow_delay + 0.5;

	if (opts >= argc) {
		fprintf(stderr, "Requires path to at least one image\n\n");
		print_help();
		return 1;
	}

	signal(SIGINT, sighandler);
	signal(SIGSEGV, sighandler);
	signal(SIGBUS, sighandler);
	signal(SIGABRT, sighandler);

	if (init_loader(&params, (const char **)argv + opts))
		return 1;

	init_backend(config.backend_init);

	if (config.emul_type != GP_PIXEL_UNKNOWN) {
		backend = GP_BackendVirtualInit(backend, config.emul_type,
		                                GP_BACKEND_CALL_EXIT);
	}

	context = backend->context;

	black_pixel = GP_ColorToContextPixel(GP_COL_BLACK, context);
	white_pixel = GP_ColorToContextPixel(GP_COL_WHITE, context);
	gray_pixel = GP_RGBToContextPixel(0x33, 0x33, 0x33, context);

	GP_Fill(context, black_pixel);
	GP_BackendFlip(backend);

	params.show_progress_once = 1;
	show_image(&params);

	if (params.sleep_ms) {
		timer.expires = params.sleep_ms;
		GP_BackendAddTimer(backend, &timer);
	}

	for (;;) {
		GP_Event ev;

		while (GP_BackendWaitEvent(backend, &ev)) {

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
				case GP_EV_REL_POS:
					if (GP_EventGetKey(&ev, GP_BTN_LEFT))
						set_zoom_offset(&params,
						                ev.val.rel.rx,
								ev.val.rel.ry);
				break;
				}
			break;
			case GP_EV_KEY:
				if (ev.code != GP_EV_KEY_DOWN)
					continue;

				switch (ev.val.key.key) {
				case GP_KEY_H:
					draw_help(backend);
					show_image(&params);
				break;
				case GP_KEY_F:
					if (GP_BackendIsX11(backend))
						GP_BackendX11RequestFullscreen(backend, 2);
				break;
				case GP_KEY_I:
				        config.show_info = !config.show_info;

					params.show_progress_once = 1;
					show_image(&params);
				break;
				case GP_KEY_P:
					config.show_progress = !config.show_progress;
				break;
				case GP_KEY_R:
					config.orientation++;

					if (config.orientation > ROTATE_270)
						config.orientation = 0;

					params.show_progress_once = 1;
					show_image(&params);
				break;
				case GP_KEY_E:
					if (config.orientation > 0)
						config.orientation--;
					else
						config.orientation = ROTATE_270;

					params.show_progress_once = 1;
					show_image(&params);
				break;
				case GP_KEY_S:
					if (params.sleep_ms) {
						if (GP_BackendTimersInQueue(backend)) {
							GP_BackendRemTimer(backend, &timer);
						} else {
							timer.expires = params.sleep_ms;
							GP_BackendAddTimer(backend, &timer);
						}
					}
				break;
				case GP_KEY_RIGHT_BRACE:
					params.resampling_method++;

					if (params.resampling_method > GP_INTERP_MAX)
						params.resampling_method = 0;
					if (params.resampling_method == GP_INTERP_CUBIC)
						params.resampling_method++;
					if (params.resampling_method == GP_INTERP_LINEAR_LF_INT) {
						params.use_low_pass = 0;
						params.show_nn_first = 0;
					} else {
						params.use_low_pass = 1;
						params.show_nn_first = 1;
					}

					params.show_progress_once = 1;
					show_image(&params);
				break;
				case GP_KEY_LEFT_BRACE:
					if (params.resampling_method == 0)
						params.resampling_method = GP_INTERP_MAX;
					else
						params.resampling_method--;
					if (params.resampling_method == GP_INTERP_CUBIC)
						params.resampling_method--;
					if (params.resampling_method == GP_INTERP_LINEAR_LF_INT) {
						params.use_low_pass = 0;
						params.show_nn_first = 0;
					} else {
						params.use_low_pass = 1;
						params.show_nn_first = 1;
					}

					params.show_progress_once = 1;
					show_image(&params);
				break;
				case GP_KEY_L:
					params.use_low_pass = !params.use_low_pass;

					params.show_progress_once = 1;
					show_image(&params);
				break;
				case GP_KEY_D:
					image_cache_drop(params.img_resized_cache);
					image_loader_drop_cache();
				break;
				case GP_KEY_ESC:
				case GP_KEY_ENTER:
				case GP_KEY_Q:
					stop_loader();
					image_cache_destroy(params.img_resized_cache);
					image_loader_destroy();
					GP_BackendExit(backend);
					return 0;
				break;
				case GP_KEY_PAGE_UP:
					params.show_progress_once = 1;
					image_seek(&params, IMG_DIR, -1);
				break;
				case GP_KEY_PAGE_DOWN:
					params.show_progress_once = 1;
					image_seek(&params, IMG_DIR, 1);
				break;
				case GP_KEY_HOME:
					params.show_progress_once = 1;
					image_seek(&params, IMG_FIRST, 0);
				break;
				case GP_KEY_END:
					params.show_progress_once = 1;
					image_seek(&params, IMG_LAST, 0);
				break;
				case GP_KEY_RIGHT:
					if (shift_flag)
						set_zoom_offset(&params, 1, 0);
					else
						set_zoom_offset(&params, 10, 0);

				break;
				case GP_KEY_LEFT:
					if (shift_flag)
						set_zoom_offset(&params, -1, 0);
					else
						set_zoom_offset(&params, -10, 0);
				break;
				case GP_KEY_UP:
					if (shift_flag)
						set_zoom_offset(&params, 0, -1);
					else
						set_zoom_offset(&params, 0, -10);
				break;
				case GP_KEY_DOWN:
					if (shift_flag)
						set_zoom_offset(&params, 0, 1);
					else
						set_zoom_offset(&params, 0, 10);
				break;
				next:
				case GP_KEY_SPACE:
					params.show_progress_once = 1;
					if (shift_flag)
						image_seek(&params, IMG_CUR, 10);
					else
						image_seek(&params, IMG_CUR, 1);
				break;
				prev:
				case GP_KEY_BACKSPACE:
					params.show_progress_once = 1;
					if (shift_flag)
						image_seek(&params, IMG_CUR, -10);
					else
						image_seek(&params, IMG_CUR, -1);
				break;
				case GP_KEY_1 ... GP_KEY_9: {
					int val = ev.val.key.key - GP_KEY_1 + 1;
					resize_backend(val, shift_flag);
				} break;
				case GP_KEY_0:
					resize_backend(10, shift_flag);
				break;
				case GP_KEY_KP_PLUS:
				case GP_KEY_DOT:
					params.show_progress_once = 1;
					zoom_mul(&params, 1.5);
				break;
				case GP_KEY_KP_MINUS:
				case GP_KEY_COMMA:
					params.show_progress_once = 1;
					zoom_mul(&params, 1/1.5);
				break;
				case GP_KEY_F1 ... GP_KEY_F10:
					image_action_run(ev.val.key.key - GP_KEY_F1,
					                 image_loader_img_path());
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
					show_image(&params);
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

	return 0;
}
