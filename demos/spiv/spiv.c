// SPDX-License-Identifier: GPL-2.1-or-later
/*
 * Copyright (C) 2009-2014 Cyril Hrubis <metan@ucw.cz>
 */

 /*

   SPIV -- Simple yet Powerful Image Viewer.

  */

#include <errno.h>
#include <signal.h>
#include <string.h>
#include <pthread.h>

#include <gfxprim.h>

#include "image_cache.h"
#include "image_list.h"
#include "image_loader.h"
#include "image_actions.h"
#include "spiv_help.h"
#include "spiv_config.h"
#include "cpu_timer.h"

static gp_pixel black_pixel;
static gp_pixel white_pixel;
static gp_pixel gray_pixel;

static gp_backend *backend = NULL;

/* image loader thread */
static int abort_flag = 0;
static int show_progress = 0;
static int loader_running = 0;

struct loader_params {
	/* current resize ratio */
	float zoom_rat;

	/* offset in pixels */
	unsigned int zoom_x_offset;
	unsigned int zoom_y_offset;

	/* flag that is turned on when user changes zoom */
	unsigned int zoom_manual;

	long show_progress_once:2;
	/* use nearest neighbour resampling first */
	long show_nn_first:4;
	/* use low pass before resampling */
	long use_low_pass:6;
	/* resampling method */
	int resampling_method;

	/* slideshow sleep */
	int sleep_ms;

	/* caches for loaded images */
	struct image_cache *img_resized_cache;
};

static int image_loader_callback(gp_progress_cb *self)
{
	static gp_size size = 0;
	gp_pixmap *c = backend->pixmap;

	if (abort_flag)
		return 1;

	if (!show_progress)
		return 0;

	char buf[100];

	snprintf(buf, sizeof(buf), "%s ... %-3.1f%%",
	         (const char*)self->priv, self->percentage);

	int align = GP_ALIGN_CENTER|GP_VALIGN_ABOVE;

	size = gp_text_width(config.style, buf);

	int start = c->w/2 - size/2 - 10;
	int end   = c->w/2 + size/2 + 10;
	int middle = start + (end - start) * self->percentage / 100;
	int top = c->h - gp_text_height(config.style) - 11;

	gp_fill_rect_xyxy(c, start, c->h - 1, middle, top, gray_pixel);
	gp_fill_rect_xyxy(c, middle, c->h - 1, end, top, black_pixel);

	gp_text(c, config.style, c->w/2, c->h - 5, align,
	        white_pixel, black_pixel, buf);

	gp_backend_update_rect(backend, start, c->h - 1, end, top);

	return 0;
}

static gp_pixmap *load_image(int elevate);

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

	gp_backend_set_caption(backend, buf);
}

/*
 * Loads image
 */
static gp_pixmap *load_image(int elevate)
{
	gp_pixmap *img;
	gp_progress_cb callback = {.callback = image_loader_callback,
	                                .priv = "Loading image"};

	img = image_loader_get_image(&callback, elevate);

	if (img)
		return img;

	gp_pixmap *pixmap = backend->pixmap;

	gp_fill(pixmap, black_pixel);
	gp_print(pixmap, config.style, pixmap->w/2, pixmap->h/2 - 10,
	         GP_ALIGN_CENTER|GP_VALIGN_CENTER, white_pixel, black_pixel,
	         "'%s'", image_loader_img_path());
	gp_print(pixmap, config.style, pixmap->w/2, pixmap->h/2 + 10,
	         GP_ALIGN_CENTER|GP_VALIGN_CENTER, white_pixel, black_pixel,
	         "Failed to load image :( (%s)", strerror(errno));
	gp_backend_flip(backend);

	return NULL;
}

/*
 * Fill pixmap with chessboard-like pattern.
 */
static void pattern_fill(gp_pixmap *pixmap, unsigned int x0, unsigned int y0,
                         unsigned int w, unsigned int h)
{
	unsigned int x, y, i, j = 0;
	gp_pixel col[2];

	col[0] = gp_rgb_to_pixmap_pixel(0x64, 0x64, 0x64, pixmap);
	col[1] = gp_rgb_to_pixmap_pixel(0x80, 0x80, 0x80, pixmap);

	unsigned int wm = w/20 < 5 ? 5 : w/20;
	unsigned int hm = h/20 < 5 ? 5 : h/20;

	for (y = 0; y < h; y += hm) {
		i = j;
		j = !j;
		for (x = 0; x < w; x += wm) {
			gp_fill_rect_xywh(pixmap, x0 + x, y0 + y, wm, hm, col[i]);
			i = !i;
		}
	}
}


static void info_printf(gp_pixmap *pixmap, gp_coord x, gp_coord y,
                        const char *fmt, ...)
                        __attribute__ ((format (printf, 4, 5)));

static void info_printf(gp_pixmap *pixmap, gp_coord x, gp_coord y,
                        const char *fmt, ...)
{
	va_list va, vac;

	va_start(va, fmt);

	va_copy(vac, va);
	gp_vprint(pixmap, config.style, x-1, y-1, GP_ALIGN_RIGHT|GP_VALIGN_BOTTOM|GP_TEXT_NOBG,
	          black_pixel, white_pixel, fmt, vac);
	va_end(vac);

	gp_vprint(pixmap, config.style, x, y, GP_ALIGN_RIGHT|GP_VALIGN_BOTTOM|GP_TEXT_NOBG,
	          white_pixel, black_pixel, fmt, va);

	va_end(va);
}

static unsigned int print_meta_data(gp_data_node *node, gp_pixmap *pixmap,
                                    unsigned int th, unsigned int y, int level)
{
	gp_data_node *i;
	unsigned int x;

	for (i = gp_data_dict_first(node); i; i = i->next) {
		y += th;
		x = th * level + 10;
		switch (i->type) {
		case GP_DATA_INT:
			info_printf(pixmap, x, y, "%s : %li", i->id, i->value.i);
		break;
		case GP_DATA_DOUBLE:
			info_printf(pixmap, x, y, "%s : %lf", i->id, i->value.d);
		break;
		case GP_DATA_STRING:
			info_printf(pixmap, x, y, "%s : %s", i->id, i->value.str);
		break;
		case GP_DATA_RATIONAL:
			info_printf(pixmap, x, y, "%s : %li/%li",
			            i->id, i->value.rat.num, i->value.rat.den);
		break;
		case GP_DATA_DICT:
			info_printf(pixmap, x, y, "%s", i->id);
			y = print_meta_data(i, pixmap, th, y, level+1);
		break;
		}
	}

	return y;
}

static void show_info(struct loader_params *params, gp_pixmap *img,
                      gp_pixmap *orig_img)
{
	gp_pixmap *pixmap = backend->pixmap;
	const char *img_path = image_loader_img_path();

	set_caption(img_path, params->zoom_rat);

	if (!config.show_info)
		return;

	gp_size th = gp_text_height(config.style), y = 10;

	info_printf(pixmap, 10, y, "%ux%u (%ux%u) 1:%3.3f %3.1f%% %s",
	         img->w, img->h, orig_img->w, orig_img->h, params->zoom_rat,
		 params->zoom_rat * 100, gp_pixel_type_name(img->pixel_type));
	y += th + 2;

	info_printf(pixmap, 10, y, "%s", img_name(img_path));

	y += th + 2;

	if (params->zoom_rat != 1.00) {
		info_printf(pixmap, 10, y, "%s%s",
		            params->use_low_pass && params->zoom_rat < 1 ? "Gaussian LP + " : "",
	                    gp_interpolation_type_name(params->resampling_method));
		y += th + 2;
	}

	unsigned int count = image_loader_count();
	unsigned int pos = image_loader_pos() + 1;

	info_printf(pixmap, 10, y, "%u of %u", pos, count);
	y += th + 2;

	if (image_loader_is_in_dir()) {
		unsigned int dir_count = image_loader_dir_count();
		unsigned int dir_pos = image_loader_dir_pos() + 1;

		info_printf(pixmap, 10, y,
			    "%u of %u in directory", dir_pos, dir_count);
	}

	gp_storage *meta_data = image_loader_get_meta_data();

	if (!meta_data)
		return;

	gp_data_node *node = gp_storage_root(meta_data);

	if (node->type != GP_DATA_DICT)
		return;

	print_meta_data(node, pixmap, th + 2, y + th, 0);
}

static void update_display(struct loader_params *params, gp_pixmap *img,
                           gp_pixmap *orig_img)
{
	gp_pixmap *pixmap = backend->pixmap;
	struct cpu_timer timer;
	gp_progress_cb callback = {.callback = image_loader_callback};

	if (abort_flag)
		return;

	switch (config.combined_orientation) {
	case ROTATE_0:
	break;
	case ROTATE_90:
		callback.priv = "Rotating image (90)";
		img = gp_filter_rotate_90_alloc(img, &callback);
	break;
	case ROTATE_180:
		callback.priv = "Rotating image (180)";
		img = gp_filter_rotate_180_alloc(img, &callback);
	break;
	case ROTATE_270:
		callback.priv = "Rotating image (270)";
		img = gp_filter_rotate_270_alloc(img, &callback);
	break;
	default:
	break;
	}

	if (img == NULL)
		return;

	int cx = 0;
	int cy = 0;

	/*
	 * Center the image, if window size is fixed and
	 * the image is smaller than window.
	 */
	if (config.win_strategy == ZOOM_WIN_FIXED) {

		if (img->w < pixmap->w)
			cx = (pixmap->w - img->w)/2;

		if (img->h < pixmap->h)
			cy = (pixmap->h - img->h)/2;
	}

	if (params->zoom_manual) {
		cx = params->zoom_x_offset;
		cy = params->zoom_y_offset;
	}

	gp_pixmap sub_display;

	if (abort_flag)
		return;

	cpu_timer_start(&timer, "Blitting");

	if (config.floyd_steinberg) {
		callback.priv = "Dithering";
		gp_sub_pixmap(pixmap, &sub_display, cx, cy, img->w, img->h);
		gp_filter_floyd_steinberg(img, &sub_display, NULL);
	//	gp_filter_hilbert_peano(img, &sub_display, NULL);
	} else {
		if (gp_pixel_has_flags(img->pixel_type, GP_PIXEL_HAS_ALPHA))
			pattern_fill(pixmap, cx, cy, img->w, img->h);
		gp_blit_clipped(img, 0, 0, img->w, img->h, pixmap, cx, cy);

	}

	cpu_timer_stop(&timer);

	/* clean up the rest of the display */
	gp_fill_rect_xywh(pixmap, 0, 0, cx, pixmap->h, black_pixel);
	gp_fill_rect_xywh(pixmap, 0, 0, pixmap->w, cy, black_pixel);


	int w = pixmap->w - img->w - cx;

	if (w > 0)
		gp_fill_rect_xywh(pixmap, img->w + cx, 0, w, pixmap->h, black_pixel);

	int h = pixmap->h - img->h - cy;

	if (h > 0)
		gp_fill_rect_xywh(pixmap, 0, img->h + cy, pixmap->w, h, black_pixel);

	show_info(params, img, orig_img);

	if (config.combined_orientation)
		gp_pixmap_free(img);


	if (abort_flag)
		return;

	gp_backend_flip(backend);
}

gp_pixmap *load_resized_image(struct loader_params *params, gp_size w, gp_size h)
{
	gp_pixmap *img, *res = NULL;
	struct cpu_timer timer;
	gp_progress_cb callback = {.callback = image_loader_callback};

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
		gp_pixmap *nn = gp_filter_resize_nn_alloc(img, w, h, NULL);
		if (nn != NULL) {
			update_display(params, nn, img);
			gp_pixmap_free(nn);
		}
	}

	/* Do low pass filter */
	if (params->use_low_pass && params->zoom_rat < 1) {
		cpu_timer_start(&timer, "Blur");
		callback.priv = "Blurring Image";

		res = gp_filter_gaussian_blur_alloc(img, 0.4/params->zoom_rat,
		                                 0.4/params->zoom_rat, &callback);

		if (res == NULL)
			return NULL;

		img = res;

		cpu_timer_stop(&timer);
	}

//	img->gamma = gp_gamma_acquire(img->pixel_type, 0.45);

	cpu_timer_start(&timer, "Resampling");
	callback.priv = "Resampling Image";
	gp_pixmap *i1 = gp_filter_resize_alloc(img, w, h, params->resampling_method, &callback);
	img = i1;
	cpu_timer_stop(&timer);

/*
	if (params->zoom_rat > 1.5) {
		cpu_timer_start(&timer, "Sharpening");
		callback.priv = "Sharpening";
		gp_filter_edge_sharpening(i1, i1, 0.1, &callback);
		cpu_timer_stop(&timer);
	}
*/

	/* Free low passed pixmap if needed */
	gp_pixmap_free(res);

	if (img == NULL)
		return NULL;

	image_cache_put2(params->img_resized_cache, img, NULL, "%s %ux%u r%i l%i",
	                 img_path, w, h, params->resampling_method,
	                 params->use_low_pass);

	return img;
}

static void exif_autorotate(void)
{
	gp_data_node *orientation;

	config.combined_orientation = config.orientation;

	if (!config.exif_autorotate)
		return;

	orientation = gp_storage_get_by_path(image_loader_get_meta_data(),
	                                      NULL, "/Exif/Orientation");
	if (!orientation)
		return;

	switch (orientation->value.i) {
	case GP_EXIF_UPPER_LEFT:
		config.combined_orientation += ROTATE_0;
	break;
	case GP_EXIF_LOWER_RIGHT:
		config.combined_orientation += ROTATE_180;
	break;
	case GP_EXIF_UPPER_RIGHT:
		config.combined_orientation += ROTATE_90;
	break;
	case GP_EXIF_LOWER_LEFT:
		config.combined_orientation += ROTATE_270;
	break;
	}

	if (config.combined_orientation > ROTATE_270)
		config.combined_orientation -= ROTATE_360;
}

static float calc_img_size(struct loader_params *params,
                           uint32_t img_w, uint32_t img_h,
                           uint32_t win_w, uint32_t win_h)
{
	float w_rat, h_rat, rat;
	unsigned int max_win_w = config.max_win_w;
	unsigned int max_win_h = config.max_win_h;

	exif_autorotate();

	switch (config.combined_orientation) {
	case ROTATE_90:
	case ROTATE_270:
		GP_SWAP(win_w, win_h);
		GP_SWAP(max_win_w, max_win_h);
	break;
	default:
	break;
	}

	if (params->zoom_manual) {
		if (config.win_strategy == ZOOM_WIN_RESIZABLE) {
			win_w = GP_MIN(max_win_w, img_w * params->zoom_rat + 0.5);
			win_h = GP_MIN(max_win_h, img_h * params->zoom_rat + 0.5);

			switch (config.combined_orientation) {
			case ROTATE_90:
			case ROTATE_270:
				gp_backend_resize(backend, win_h, win_w);
			break;
			default:
				gp_backend_resize(backend, win_w, win_h);
			}
		}
		return params->zoom_rat;
	}


	if (config.win_strategy == ZOOM_WIN_RESIZABLE) {
		win_w = GP_MIN(max_win_w, img_w);
		win_h = GP_MIN(max_win_h, img_h);

		/*
		 * Image is larger than screen and downscale is enabled ->
		 * resize window to match image ratio.
		 */
		if ((win_w != img_w || win_h != img_h) &&
		    config.zoom_strategy & ZOOM_IMAGE_DOWNSCALE) {

			w_rat = 1.00 * win_w / img_w;
			h_rat = 1.00 * win_h / img_h;

			if (w_rat > 1)
				w_rat = 1;

			if (h_rat > 1)
				w_rat = 1;

			rat = GP_MIN(h_rat, w_rat);

			win_w = rat * img_w + 0.5;
			win_h = rat * img_h + 0.5;
		}

		switch (config.combined_orientation) {
		case ROTATE_90:
		case ROTATE_270:
			gp_backend_resize(backend, win_h, win_w);
		break;
		default:
			gp_backend_resize(backend, win_w, win_h);
		}
	}

	if (img_w <= win_w && img_h <= win_h) {
		if (!(config.zoom_strategy & ZOOM_IMAGE_UPSCALE))
			return 1.00;
	} else {
		if (!(config.zoom_strategy & ZOOM_IMAGE_DOWNSCALE))
			return 1.00;

	}

	w_rat = 1.00 * win_w / img_w;
	h_rat = 1.00 * win_h / img_h;

	return GP_MIN(w_rat, h_rat);
}

static void *image_loader(void *ptr)
{
	struct loader_params *params = ptr;
	struct cpu_timer sum_timer;
	gp_pixmap *img, *orig_img, *pixmap = backend->pixmap;

	cpu_timer_start(&sum_timer, "sum");

	show_progress = config.show_progress || params->show_progress_once;
	params->show_progress_once = 0;

	if ((orig_img = load_image(0)) == NULL) {
		loader_running = 0;
		return NULL;
	}

	/* Figure zoom */
	gp_size w, h;

	params->zoom_rat = calc_img_size(params, orig_img->w, orig_img->h,
	                                 pixmap->w, pixmap->h);

	w = orig_img->w * params->zoom_rat + 0.5;
	h = orig_img->h * params->zoom_rat + 0.5;

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
		gp_backend_exit(backend);
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
	params->zoom_manual = 0;
	image_loader_seek(offset, whence);
	show_image(params);
}

static void set_zoom_offset(struct loader_params *params, int dx, int dy)
{
	params->zoom_manual = 1;
	params->zoom_x_offset += dx;
	params->zoom_y_offset += dy;
	show_image(params);
}

static void zoom_mul(struct loader_params *params, float mul)
{
	params->zoom_manual = 1;
	params->zoom_rat *= mul;
	show_image(params);
}

static void zoom_set(struct loader_params *params, float mul)
{
	params->zoom_manual = 1;
	params->zoom_rat = mul;
	show_image(params);
}

#define PRINT(msg) do { \
	if (write(STDERR_FILENO, msg, sizeof(msg) - 1)) { \
		/* gcc silencer */ \
	} \
} while (0)

static void sighandler(int signo)
{
	(void) signo;

	if (backend)
		gp_backend_exit(backend);

	PRINT("Got signal\n");

	_exit(1);
}

static void init_backend(const char *backend_opts)
{
	backend = gp_backend_init(backend_opts, "Spiv");

	if (backend == NULL) {
		fprintf(stderr, "Failed to initalize backend '%s'\n", backend_opts);
		exit(1);
	}

	if (config.full_screen)
		gp_backend_fullscreen(backend, 2);
}

#define RESIZED_CACHE_MAX 400 * 1024
#define ORIG_CACHE_MAX 80 * 1024

/*
 * Figure out cache size depending on the size of RAM.
 *
 * Initialize cache, image loader.
 */
static int init_loader(struct loader_params *params, const char **argv)
{
	size_t size = image_cache_get_ram_size();
	size_t resized_size = size/10;
	size_t orig_size = size/50;

	if (resized_size > RESIZED_CACHE_MAX)
		resized_size = RESIZED_CACHE_MAX;

	if (orig_size > ORIG_CACHE_MAX)
		orig_size = ORIG_CACHE_MAX;

	GP_DEBUG(1, "Resized cache size = %zukB", resized_size);
	GP_DEBUG(1, "Orig cache size = %zukB", orig_size);

	if (image_loader_init(argv, orig_size))
		return 1;

	params->img_resized_cache = image_cache_create(resized_size);

	return 0;
}

static uint32_t timer_callback(gp_timer *self)
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
	gp_pixmap *pixmap = NULL;
	int shift_flag;
	int opts;

	struct loader_params params = {
		.show_progress_once = 0,
		.show_nn_first = 0,
		.resampling_method = GP_INTERP_LINEAR_LF_INT,

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

		if (!strcmp(config.backend_init, "help")) {
			init_backend(config.backend_init);
			return 0;
		}

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
		backend = gp_backend_virt_init(backend, config.emul_type,
		                               GP_BACKEND_CALL_EXIT);
	}

	pixmap = backend->pixmap;

	black_pixel = gp_rgb_to_pixmap_pixel(0x00, 0x00, 0x00, pixmap);
	white_pixel = gp_rgb_to_pixmap_pixel(0xff, 0xff, 0xff, pixmap);
	gray_pixel = gp_rgb_to_pixmap_pixel(0x33, 0x33, 0x33, pixmap);

	gp_fill(pixmap, black_pixel);
	gp_backend_flip(backend);

	params.show_progress_once = 1;
	show_image(&params);

	if (params.sleep_ms) {
		timer.expires = params.sleep_ms;
		gp_backend_add_timer(backend, &timer);
	}

	for (;;) {
		gp_event *ev;

		while ((ev = gp_backend_wait_event(backend))) {

			shift_flag = gp_backend_key_pressed(backend, GP_KEY_LEFT_SHIFT) ||
			             gp_backend_key_pressed(backend, GP_KEY_RIGHT_SHIFT);

			switch (ev->type) {
			case GP_EV_REL:
				switch (ev->code) {
				case GP_EV_REL_WHEEL:
					if (ev->val > 0)
						goto next;

					if (ev->val < 0)
						goto prev;
				break;
				case GP_EV_REL_POS:
					if (gp_backend_key_pressed(backend, GP_BTN_LEFT)) {
						set_zoom_offset(&params,
						                ev->rel.rx,
								ev->rel.ry);
					}
				break;
				}
			break;
			case GP_EV_KEY:
				if (ev->code != GP_EV_KEY_DOWN)
					continue;

				switch (ev->key.key) {
				case GP_KEY_H:
					draw_help(backend);
					show_image(&params);
				break;
				case GP_KEY_F:
					gp_backend_fullscreen(backend, 2);
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
						config.orientation = ROTATE_0;

					params.show_progress_once = 1;
					show_image(&params);
				break;
				case GP_KEY_E:
					if (config.orientation > ROTATE_0)
						config.orientation--;
					else
						config.orientation = ROTATE_270;

					params.show_progress_once = 1;
					show_image(&params);
				break;
				case GP_KEY_S:
					if (params.sleep_ms) {
						if (gp_backend_timers_in_queue(backend)) {
							gp_backend_rem_timer(backend, &timer);
						} else {
							timer.expires = params.sleep_ms;
							gp_backend_add_timer(backend, &timer);
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
				case GP_KEY_C:
					image_cache_drop(params.img_resized_cache);
					image_loader_drop_cache();
				break;
				case GP_KEY_W:
					config_win_toggle();
					params.show_progress_once = 1;
					show_image(&params);
				break;
				case GP_KEY_U:
					config_upscale_toggle();
					params.show_progress_once = 1;
					show_image(&params);
				break;
				case GP_KEY_D:
					config_downscale_toggle();
					params.show_progress_once = 1;
					show_image(&params);
				break;
				case GP_KEY_ESC:
				case GP_KEY_ENTER:
				case GP_KEY_Q:
					stop_loader();
					image_cache_destroy(params.img_resized_cache);
					image_loader_destroy();
					gp_backend_exit(backend);
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
					float val = ev->key.key - GP_KEY_1 + 1;

					if (!shift_flag)
						val = 1/val;

					zoom_set(&params, val);
				} break;
				case GP_KEY_0:
					if (shift_flag)
						zoom_set(&params, 10);
					else
						zoom_set(&params, 0.1);
				break;
				case GP_KEY_KP_PLUS:
				case GP_KEY_DOT:
					params.show_progress_once = 1;
					if (shift_flag)
						zoom_mul(&params, 1.1);
					else
						zoom_mul(&params, 1.5);
				break;
				case GP_KEY_KP_MINUS:
				case GP_KEY_COMMA:
					params.show_progress_once = 1;
					if (shift_flag)
						zoom_mul(&params, 1/1.1);
					else
						zoom_mul(&params, 1/1.5);
				break;
				case GP_KEY_F1 ... GP_KEY_F10:
					image_action_run(ev->key.key - GP_KEY_F1 + 1,
					                 image_loader_img_path());
				break;
				}
			break;
			case GP_EV_SYS:
				switch (ev->code) {
				case GP_EV_SYS_RESIZE:
					/* stop loader thread before resizing backend buffer */
					stop_loader();
					gp_backend_resize_ack(backend);
					gp_fill(backend->pixmap, 0);
					params.show_progress_once = 1;
					show_image(&params);
				break;
				case GP_EV_SYS_QUIT:
					gp_backend_exit(backend);
					return 0;
				break;
				}
			break;
			}
		}
	}

	return 0;
}
