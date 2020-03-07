// SPDX-License-Identifier: GPL-2.1-or-later
/*
 * Copyright (C) 2009-2012 Cyril Hrubis <metan@ucw.cz>
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#include "gfxprim.h"

#include "params.h"
#include "histogram.h"

static gp_progress_cb *progress_callback = NULL;

static const char *progress_prefix = NULL;

static int show_progress(gp_progress_cb *self)
{
	fprintf(stderr, "\r%s %3.2f%%",
	        progress_prefix, self->percentage);

	return 0;
}

static int param_err(const struct param *self, const char *val, void *priv)
{
	/* invalid parameter name */
	if (self == NULL) {
		fprintf(stderr, "ERROR: %s: Unknown parameter name '%s'\n",
		                (char*)priv, val);
		return 1;
	}

	/* just regular error */
	fprintf(stderr, "ERROR: %s: Invalid %s parameter value %s = '%s'",
	        (char *)priv, param_type_name(self->type), self->name, val);

	if (self->type == PARAM_ENUM) {
		unsigned int i;

		fprintf(stderr, " is not in [");

		for (i = 0; self->enum_table[i] != NULL; i++)
			if (self->enum_table[i+1] == NULL)
				fprintf(stderr, "'%s']", self->enum_table[i]);
			else
				fprintf(stderr, "'%s' | ", self->enum_table[i]);
	}

	fprintf(stderr, "\n");

	return 1;
}

static void print_error(const char *error)
{
	fprintf(stderr, "ERROR: %s\n", error);
}

/* resize filter */

static const char *resize_algs[] = {
	"nn",
	"linear-int",
	"linear-lf-int",
	"cubic",
	"cubic-int",
	NULL
};

static int resize_check_ratio(const struct param *self __attribute__((unused)),
                              void *val, int count __attribute__((unused)))
{
	float f = *((float*)val);

	if (f <= 0)
		return -1;

	return 0;
}

static struct param resize_params[] = {
	{"alg",   PARAM_ENUM,  "algorithm to be used", resize_algs, NULL},
	{"ratio", PARAM_FLOAT, "scale ratio", NULL, resize_check_ratio},
	{NULL,    0,           NULL,          NULL, NULL}
};

static int resize(gp_pixmap **c, const char *params)
{
	int alg = 1;
	float ratio = -1;

	if (param_parse(params, resize_params, "resize", param_err,
	                &alg, &ratio))
		return EINVAL;

	if (ratio == -1) {
		print_error("resize: ratio parameter is missing");
		return EINVAL;
	}

	gp_size w = ratio * (*c)->w;
	gp_size h = ratio * (*c)->h;
	gp_pixmap *res = NULL;

	res = gp_filter_resize_alloc(*c, w, h, alg, progress_callback);

	if (res == NULL)
		return EINVAL;

	gp_pixmap_free(*c);
	*c = res;

	return 0;
}

/* scale filter */

static const char *scale_algs[] = {
	"nn",
	"linear-int",
	"linear-lf-int",
	"cubic",
	"cubic-int",
	NULL
};

static int scale_check_size(const struct param *self __attribute__((unused)),
                            void *val, int count __attribute__((unused)))
{
	int i = *((int*)val);

	if (i <= 0)
		return 1;

	return 0;
}

static struct param scale_params[] = {
	{"alg", PARAM_ENUM, "algorithm to be used", scale_algs, NULL},
	{"w", PARAM_INT, "new width (only width may be passed)",  NULL, scale_check_size},
	{"h", PARAM_INT, "new height (only height may be passed)", NULL, scale_check_size},
	{NULL,  0,           NULL,        NULL, NULL}
};

static int scale(gp_pixmap **c, const char *params)
{
	int alg = 1;
	int w = -1;
	int h = -1;

	if (param_parse(params, scale_params, "scale", param_err,
	                &alg, &w, &h))
		return EINVAL;

	if (w == -1 && h == -1) {
		print_error("scale: w and/or h missing");
		return EINVAL;
	}

	if (w == -1)
		w = (*c)->w * (1.00 * h/(*c)->h) + 0.5;

	if (h == -1)
		h = (*c)->h * (1.00 * w/(*c)->w) + 0.5;

	gp_pixmap *res = NULL;

	res = gp_filter_resize_alloc(*c, w, h, alg, progress_callback);

	if (res == NULL)
		return EINVAL;

	gp_pixmap_free(*c);
	*c = res;

	return 0;
}

/* rotate filter */

static const char *rotate_rots[] = {
	"90",
	"180",
	"270",
};

static struct param rotate_params[] = {
	{"rot", PARAM_ENUM,  "image rotation", rotate_rots, NULL},
	{NULL,  0,           NULL,        NULL,             NULL}
};

static int rotate(gp_pixmap **c, const char *params)
{
	int rot = -1;

	if (param_parse(params, rotate_params, "rotate", param_err, &rot))
		return EINVAL;

	if (rot == -1) {
		print_error("rotate: rot parameter is missing");
		return EINVAL;
	}

	gp_pixmap *res = NULL;

	switch (rot) {
	case 0:
		res = gp_filter_rotate_90_alloc(*c, progress_callback);
	break;
	case 1:
		res = gp_filter_rotate_180_alloc(*c, progress_callback);
	break;
	case 2:
		res = gp_filter_rotate_270_alloc(*c, progress_callback);
	break;
	}

	if (res == NULL)
		return ENOMEM;

	gp_pixmap_free(*c);
	*c = res;

	return 0;
}

/* mirror filter */

static struct param mirror_params[] = {
	{"vert",  PARAM_BOOL,  "mirror vertically",   NULL, NULL},
	{"horiz", PARAM_BOOL,  "mirror horizontally", NULL, NULL},
	{NULL,    0,           NULL,                  NULL, NULL}
};

static int mirror(gp_pixmap **c, const char *params)
{
	int vert = 0, horiz = 0;

	if (param_parse(params, mirror_params, "mirror", param_err, &vert, &horiz))
		return EINVAL;

	if (vert)
		gp_filter_mirror_v(*c, *c, progress_callback);

	if (horiz)
		gp_filter_mirror_h(*c, *c, progress_callback);

	return 0;
}

/* brightness filter */

static struct param bright_params[] = {
	{"inc", PARAM_FLOAT, "brightness increment",                NULL, NULL},
	{NULL,  0,         NULL,                                  NULL, NULL}
};

static int bright(gp_pixmap **c, const char *params)
{
	float bright = 0;

	if (param_parse(params, bright_params, "bright", param_err, &bright))
		return EINVAL;

	gp_filter_brightness(*c, *c, bright, progress_callback);

	return 0;
}

/* contrast */

static struct param contrast_params[] = {
	{"mul", PARAM_FLOAT, "contrast (1.5 = +50%, 0.5 = -50%)", NULL, NULL},
	{NULL,  0,           NULL,                    NULL, NULL}
};

static int contrast(gp_pixmap **c, const char *params)
{
	float mul = 0;

	if (param_parse(params, contrast_params, "contrast", param_err, &mul))
		return EINVAL;

	if (mul <= 0) {
		print_error("contrast: mul parameter must be >= 0");
		return EINVAL;
	}

	gp_filter_contrast(*c, *c, mul, progress_callback);

	return 0;
}

/* invert */

static struct param invert_params[] = {
	{NULL,  0, NULL, NULL, NULL}
};

static int invert(gp_pixmap **c, const char *params)
{
	if (param_parse(params, invert_params, "invert", param_err))
		return EINVAL;

	gp_filter_invert(*c, *c, progress_callback);

	return 0;
}

/* blur */

static struct param blur_params[] = {
	{"sigma", PARAM_FLOAT, "sigma parameter, radii of blur (sets both)", NULL, NULL},
	{"sigma_x", PARAM_FLOAT, "sigma parameter for horizontal direction", NULL, NULL},
	{"sigma_y", PARAM_FLOAT, "sigma parameter for vertical direction", NULL, NULL},
	{NULL,  0, NULL, NULL, NULL}
};

static int blur(gp_pixmap **c, const char *params)
{
	float sigma = 0;
	float sigma_x = 0;
	float sigma_y = 0;

	if (param_parse(params, blur_params, "blur", param_err, &sigma, &sigma_x, &sigma_y))
		return EINVAL;

	if (sigma > 0) {
		sigma_x = sigma;
		sigma_y = sigma;
	}

	if (sigma_x <= 0 && sigma_y <= 0) {
		print_error("blur: at least one of sigma_x and sigma_y must be >= 0");
		return EINVAL;
	}

	gp_filter_gaussian_blur(*c, *c, sigma_x, sigma_y, progress_callback);

	return 0;
}

/* dithering */

//TODO: this should be generated
static const char *dither_formats[] = {
	"g1",
	"g2",
	"g4",
	"g8",
	"rgb565",
	"rgb666",
	NULL,
};

static const gp_pixel_type dither_pixel_types[] = {
	GP_PIXEL_G1,
	GP_PIXEL_G2,
	GP_PIXEL_G4,
	GP_PIXEL_G8,
	GP_PIXEL_RGB565,
	GP_PIXEL_RGB666,
};

static struct param dither_params[] = {
	{"format", PARAM_ENUM, "pixel type to be used", dither_formats, NULL},
	{NULL,  0, NULL, NULL, NULL}
};

static int dither(gp_pixmap **c, const char *params)
{
	int fmt = -1;

	if (param_parse(params, dither_params, "dither", param_err, &fmt))
		return EINVAL;

	if (fmt == -1) {
		print_error("dither: invalid format or format param missing");
		return EINVAL;
	}

	gp_pixmap *bw;
	bw = gp_filter_floyd_steinberg_alloc(*c, dither_pixel_types[fmt],
	                                  progress_callback);

	//TODO: so far we convert the pixmap back to RGB888
	//(so we can do further work with it)
	gp_blit(bw, 0, 0, gp_pixmap_w(bw), gp_pixmap_h(bw), *c, 0, 0);

	gp_pixmap_free(bw);

	return 0;
}

/* jpg save filter */

static struct param save_jpg_params[] = {
	{"file", PARAM_STR, "Filename to save the result", NULL, NULL},
	{NULL,  0, NULL, NULL, NULL}
};

static int save_jpg(gp_pixmap **c, const char *params)
{
	char *file = NULL;

	if (param_parse(params, save_jpg_params, "jpg", param_err, &file))
		return EINVAL;

	if (file == NULL) {
		print_error("jpg: filename missing");
		return EINVAL;
	}

	gp_save_jpg(*c, file, progress_callback);

	return 0;
}

/* png save filter */

static struct param save_png_params[] = {
	{"file", PARAM_STR, "Filename to save the result", NULL, NULL},
	{NULL,  0, NULL, NULL, NULL}
};

static int save_png(gp_pixmap **c, const char *params)
{
	char *file = NULL;

	if (param_parse(params, save_png_params, "png", param_err, &file))
		return EINVAL;

	if (file == NULL) {
		print_error("png: filename missing");
		return EINVAL;
	}

	gp_save_png(*c, file, progress_callback);

	return 0;
}

/* median filter */

static struct param median_params[] = {
	{"radius", PARAM_INT, "median radius for both x and y", NULL, NULL},
	{"radius_x", PARAM_INT, "median radius for x", NULL, NULL},
	{"radius_y", PARAM_INT, "median radius for y", NULL, NULL},
	{NULL,  0, NULL, NULL, NULL}
};

static int median(gp_pixmap **c, const char *params)
{
	int rad = -1, rad_x, rad_y;

	if (param_parse(params, median_params, "median", param_err, &rad, &rad_x, &rad_y))
		return EINVAL;

	if (rad != -1) {
		rad_x = rad;
		rad_y = rad;
	}

	if (rad_x < 0 || rad_y < 0)
		return EINVAL;

	gp_pixmap *ret = gp_filter_median_alloc(*c, rad_x, rad_y, progress_callback);

	if (ret == NULL)
		return ENOMEM;

	gp_pixmap_free(*c);
	*c = ret;

	return 0;
}

/* sigma mean filter */

static struct param sigma_mean_params[] = {
	{"radius", PARAM_INT, "median radius for both x and y", NULL, NULL},
	{"min", PARAM_INT, "minimal number of pixels to use for the mean", NULL, NULL},
	{"sigma", PARAM_FLOAT, "sigma scaled to [0,1] interval", NULL, NULL},
	{"radius_x", PARAM_INT, "median radius for x", NULL, NULL},
	{"radius_y", PARAM_INT, "median radius for y", NULL, NULL},
	{NULL,  0, NULL, NULL, NULL}
};

static int sigma_mean(gp_pixmap **c, const char *params)
{
	int rad = -1, rad_x, rad_y, min = 0;
	float sigma = 0.1;

	if (param_parse(params, sigma_mean_params, "sigma", param_err,
	                &rad, &min, &sigma, &rad_x, &rad_y))
		return EINVAL;

	if (rad != -1) {
		rad_x = rad;
		rad_y = rad;
	}

	if (rad_x < 0 || rad_y < 0)
		return EINVAL;

	(*c)->gamma = gp_gamma_acquire((*c)->pixel_type, 1.2);

	gp_pixmap *ret = gp_filter_sigma_alloc(*c, rad_x, rad_y, min, sigma, progress_callback);

	if (ret == NULL)
		return ENOMEM;

	gp_pixmap_free(*c);
	*c = ret;

	return 0;
}

/* laplacian edge sharpening filter */

static struct param sharpen_params[] = {
	{"weight", PARAM_FLOAT, "sharpening weight from [0,1] interval", NULL, NULL},
	{NULL,  0, NULL, NULL, NULL}
};

static int sharpen(gp_pixmap **c, const char *params)
{
	float weight = 0.1;

	if (param_parse(params, sharpen_params, "sigma", param_err, &weight))
		return EINVAL;

	gp_pixmap *ret = gp_filter_edge_sharpening_alloc(*c, weight, progress_callback);

	if (ret == NULL)
		return ENOMEM;

	gp_pixmap_free(*c);
	*c = ret;

	return 0;
}

/* gaussian additive noise filter */

static struct param gauss_noise_params[] = {
	{"sigma", PARAM_FLOAT, "sigma: amount of noise between [0,1]", NULL, NULL},
	{"mu", PARAM_FLOAT, "mu: offset of noise between [0,1]", NULL, NULL},
	{NULL,  0, NULL, NULL, NULL}
};

static int gauss_noise(gp_pixmap **c, const char *params)
{
	float sigma = 0.1;
	float mu = 0;

	if (param_parse(params, gauss_noise_params, "gaussian noise", param_err, &sigma, &mu))
		return EINVAL;

	gp_filter_gaussian_noise_add(*c, *c, sigma, mu, progress_callback);

	return 0;
}

/* arithmetics */

static const char *arithmetic_ops[] = {
	"difference",
	"addition",
	"multiply",
	"min",
	"max",
	NULL
};

static struct param arithmetic_params[] = {
	{"file", PARAM_STR, "Filename of image to use.", NULL, NULL},
	{"op",  PARAM_ENUM, "Arithmetic peration", arithmetic_ops, NULL},
	{NULL,  0, NULL, NULL, NULL}
};

static int arithmetic(gp_pixmap **c, const char *params)
{
	char *file = NULL;
	int op = -1;

	if (param_parse(params, arithmetic_params, "arithmetic", param_err, &file, &op))
		return EINVAL;

	if (file == NULL) {
		print_error("arithmetic: Filename missing");
		return EINVAL;
	}

	gp_pixmap *img, *res = NULL;

	if ((img = gp_load_image(file, progress_callback)) == NULL) {
		print_error("arithmetic: Invalid image.");
		return EINVAL;
	}

	switch (op) {
	case 0:
		res = gp_filter_diff_alloc(*c, img, progress_callback);
	break;
	case 1:
		res = gp_filter_add_alloc(*c, img, progress_callback);
	break;
	case 2:
		res = gp_filter_mul_alloc(*c, img, progress_callback);
	break;
	case 3:
		res = gp_filter_min_alloc(*c, img, progress_callback);
	break;
	case 4:
		res = gp_filter_max_alloc(*c, img, progress_callback);
	break;
	}

	if (res == NULL)
		return ENOMEM;

	gp_pixmap_free(*c);

	*c = res;

	return 0;
}

/* histogram */

static struct param histogram_params[] = {
	{"file", PARAM_STR, "Filename of image to use.", NULL, NULL},
	{NULL,  0, NULL, NULL, NULL}
};

static int histogram(gp_pixmap **c, const char *params)
{
	char *file = "histogram.png";

	if (param_parse(params, histogram_params, "histogram", param_err, &file))
		return EINVAL;

	if (file == NULL) {
		print_error("histogram: Filename missing");
		return EINVAL;
	}

	histogram_to_png(*c, file);
	return 0;
}

/* filters */

struct filter {
	const char *name;
	const char *desc;
	struct param *param_desc;
	int (*apply)(gp_pixmap **c, const char *params);
};

static struct filter filter_table[] = {
	{"rotate",     "rotate image", rotate_params, rotate},
	{"mirror",     "mirror vertically/horizontally", mirror_params, mirror},
	{"scale",      "scale image to given width and height", scale_params,  scale},
	{"resize",     "resize image by given ratio", resize_params, resize},
	{"bright",     "alter image brightness", bright_params, bright},
	{"contrast",   "alter image contrast", contrast_params, contrast},
	{"invert",     "inverts image", invert_params, invert},
	{"blur",       "gaussian blur", blur_params, blur},
	{"dither",     "dithers bitmap", dither_params, dither},
	{"arithmetic", "arithmetic operation", arithmetic_params, arithmetic},
	{"histogram",  "save histogram into image file", histogram_params, histogram},
	{"median",     "median filter", median_params, median},
	{"sigma",      "sigma (mean) filter", sigma_mean_params, sigma_mean},
	{"sharpen",    "laplacian edge sharpening", sharpen_params, sharpen},
	{"gauss_noise", "additive gaussian (normaly distributed) noise", gauss_noise_params, gauss_noise},
	{"jpg",        "save jpg image", save_jpg_params, save_jpg},
	{"png",        "save png image", save_png_params, save_png},
	{NULL, NULL,   NULL, NULL}
};

static struct filter *get_filter(const char *name)
{
	unsigned int i;

	for (i = 0; filter_table[i].name != NULL; i++) {
		if (!strcasecmp(filter_table[i].name, name))
			return &filter_table[i];
	}

	return NULL;
}

static void print_filter_help(void)
{
	unsigned int i, j;

	for (i = 0; filter_table[i].name != NULL; i++) {
		printf("%s\n", filter_table[i].name);

		j = strlen(filter_table[i].name);

		while (j--)
			putchar('-');
		putchar('\n');

		printf("* %s\n", filter_table[i].desc);
		putchar('\n');

		param_describe(filter_table[i].param_desc, " ");
		putchar('\n');
	}
}

/* application */

#define FILTERS_MAX 255

static const char *filter_params[FILTERS_MAX];
static const struct filter *filters[FILTERS_MAX];
static unsigned int filter_cnt = 0;

static void add_filter(char *params)
{
	if (filter_cnt >= FILTERS_MAX) {
		fprintf(stderr, "Maximal number of filters exceeded (%u), "
		                "increase and recompile.", FILTERS_MAX);
		exit(1);
	}

	const char *name = strsep(&params, ":");

	filters[filter_cnt] = get_filter(name);

	if (filters[filter_cnt] == NULL) {
		fprintf(stderr, "Invalid filter name '%s'\n", name);
		exit(1);
	}

	filter_params[filter_cnt++] = params;
}

static void apply_filters(gp_pixmap **src)
{
	unsigned int i;
	int ret;

	for (i = 0; i < filter_cnt; i++) {
		char buf[255];

		snprintf(buf, sizeof(buf), "Filter %s", filters[i]->name);

		progress_prefix = buf;

		if ((ret = filters[i]->apply(src, filter_params[i]))) {
			fprintf(stderr, "Error: %s\n", strerror(ret));
			exit(1);
		}

		if (progress_callback != NULL)
			fprintf(stderr, " done\n");
	}
}

static const char *app_help = {
	"                                                      \n"
	"     <<<<<<<<<<  Bitmap  Grinder  >>>>>>>>>>>         \n"
	"                                                      \n"
	"                +~+-----+                             \n"
	"               /| |  +-+| .11.                        \n"
	"            +-{  D|  |010101011.                      \n"
	"            |  \\| |  +-.0100101.                      \n"
	"          O=+   +~+-----+ .10110101.                  \n"
	"                            .010101.                  \n"
	"                              .1.                     \n"
	"                                                      \n"
	"                 Program options                      \n"
	"                 ===============                      \n"
	"                                                      \n"
	"-h        - prints this help                          \n"
	"-p        - show filter progress                      \n"
	"-v int    - sets gfxprim verbosity level              \n"
	"-o fmt    - output format, ppm, jpg, png              \n"
	"-f params - apply filter, multiple filters may be used\n"
	"                                                      \n"
	"                  Example usage                       \n"
	"                  =============                       \n"
	"                                                      \n"
	" grider -f resize:ratio=1.5 -f contrast:mul=1.2 in.png\n"
	"                                                      \n"
	" * will resize image 1.5 times and increases contrast \n"
	"   by 20%. The result is, just for now, saved to      \n"
	"   out_X.ppm where X is number which is increased for \n"
	"   each image given as parameter.                     \n"
	"                                                      \n"
	"                 List of filters                      \n"
	"                 ===============                      \n"
};

static void print_help(void)
{
	puts(app_help);
	print_filter_help();
}

static const char *out_fmts[] = {
	"ppm",
	"jpg",
	"png",
	NULL
};

static void check_fmt(const char *fmt)
{
	unsigned int i;

	for (i = 0; out_fmts[i] != NULL; i++)
		if (!strcmp(out_fmts[i], fmt))
			return;

	fprintf(stderr, "Invalid output format '%s'\n", fmt);
	exit(1);
}

static void save_by_fmt(struct gp_pixmap *bitmap, const char *name, const char *fmt)
{
	int ret;

	progress_prefix = "Saving Image";

	if (!strcmp(fmt, "ppm"))
		ret = gp_save_ppm(bitmap, name, progress_callback);
	else if (!strcmp(fmt, "jpg"))
		ret = gp_save_jpg(bitmap, name, progress_callback);
	else if (!strcmp(fmt, "png"))
		ret = gp_save_png(bitmap, name, progress_callback);
	else {
		printf("Invalid format %s\n", fmt);
		exit(1);
	}

	if (ret) {
		fprintf(stderr, "Failed to save bitmap: %s\n",
		        strerror(errno));
		exit(1);
	}

	if (progress_callback != NULL)
		fprintf(stderr, " done\n");
}

int main(int argc, char *argv[])
{
	gp_pixmap *bitmap;
	int opt, i;
	const char *out_fmt = "ppm";

	gp_progress_cb callback = {
		.callback = show_progress,
	};

	while ((opt = getopt(argc, argv, "f:ho:pv:")) != -1) {
		switch (opt) {
		case 'h':
			print_help();
			return 0;
		break;
		case 'v':
			i = atoi(optarg);

			if (i == 0) {
				fprintf(stderr, "ERROR: invalid debug level "
				                "'%s', expected number > 0\n",
						optarg);
				return 1;
			}

			gp_set_debug_level(i);
		break;
		case 'o':
			out_fmt = optarg;
			check_fmt(out_fmt);
		break;
		case 'f':
			add_filter(optarg);
		break;
		case 'p':
			progress_callback = &callback;
		break;
		default:
			print_help();
			return 1;
		}
	}

	if (optind >= argc) {
		fprintf(stderr, "ERROR: Expected bitmap filenames\n");
		return 1;
	}

	for (i = optind; i < argc; i++) {
		char buf[255];

		snprintf(buf, sizeof(buf), "out_%03i.%s", i - optind + 1, out_fmt);
		fprintf(stderr, "Processing '%s' -> '%s'\n", argv[i], buf);

		progress_prefix = "Loading image";

		if ((bitmap = gp_load_image(argv[i], progress_callback)) == NULL) {
			fprintf(stderr, "Failed to load bitmap: %s\n", strerror(errno));
			return 1;
		}

		if (progress_callback != NULL)
			fprintf(stderr, " done\n");

		apply_filters(&bitmap);

		save_by_fmt(bitmap, buf, out_fmt);
	}

	return 0;
}
