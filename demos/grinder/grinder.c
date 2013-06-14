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

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#include "GP.h"

#include "params.h"
#include "histogram.h"

static GP_ProgressCallback *progress_callback = NULL;

static const char *progress_prefix = NULL;

static int show_progress(GP_ProgressCallback *self)
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

static int resize(GP_Context **c, const char *params)
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

	GP_Size w = ratio * (*c)->w;
	GP_Size h = ratio * (*c)->h;
	GP_Context *res = NULL;

	res = GP_FilterResize(*c, NULL, alg, w, h, progress_callback);
	
	if (res == NULL)
		return EINVAL;

	GP_ContextFree(*c);
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

static int scale(GP_Context **c, const char *params)
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

	GP_Context *res = NULL;

	res = GP_FilterResize(*c, NULL, alg, w, h, progress_callback);

	if (res == NULL)
		return EINVAL;

	GP_ContextFree(*c);
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

static int rotate(GP_Context **c, const char *params)
{
	int rot = -1;

	if (param_parse(params, rotate_params, "rotate", param_err, &rot))
		return EINVAL;

	if (rot == -1) {
		print_error("rotate: rot parameter is missing");
		return EINVAL;
	}

	GP_Context *res = NULL;

	switch (rot) {
	case 0:
		res = GP_FilterRotate90Alloc(*c, progress_callback);
	break;
	case 1:
		res = GP_FilterRotate180Alloc(*c, progress_callback);
	break;
	case 2:
		res = GP_FilterRotate270Alloc(*c, progress_callback);
	break;
	}
		
	if (res == NULL)
		return ENOMEM;
		
	GP_ContextFree(*c);
	*c = res;

	return 0;
}

/* mirror filter */

static struct param mirror_params[] = {
	{"vert",  PARAM_BOOL,  "mirror vertically",   NULL, NULL},
	{"horiz", PARAM_BOOL,  "mirror horizontally", NULL, NULL},
	{NULL,    0,           NULL,                  NULL, NULL}
};

static int mirror(GP_Context **c, const char *params)
{
	int vert = 0, horiz = 0;

	if (param_parse(params, mirror_params, "mirror", param_err, &vert, &horiz))
		return EINVAL;

	if (vert)
		GP_FilterMirrorV(*c, *c, progress_callback);
	
	if (horiz)
		GP_FilterMirrorH(*c, *c, progress_callback);

	return 0;
}

/* brightness filter */

static struct param bright_params[] = {
	{"inc", PARAM_INT, "brightness increment",                NULL, NULL},
	{"chann", PARAM_STR, "Channel name {R, G, B, A, V, ...}", NULL, NULL},
	{NULL,  0,         NULL,                                  NULL, NULL}
};

static int bright(GP_Context **c, const char *params)
{
	int bright = 0;
	char *chann = NULL;

	if (param_parse(params, bright_params, "bright", param_err, &bright, &chann))
		return EINVAL;

	if (bright == 0) {
		print_error("bright: bright parameter is zero or missing");
		return EINVAL;
	}

	GP_FILTER_PARAMS((*c)->pixel_type, filter_params);
	
	if (chann == NULL) {
		GP_FilterParamSetIntAll(filter_params, bright);
	} else {
		GP_FilterParam *param = GP_FilterParamChannel(filter_params, chann);
	
		if (param == NULL) {
			print_error("bright: Invalid channel name");
			return EINVAL;
		}

		GP_FilterParamSetIntAll(filter_params, 0);
		param->val.i = bright;
	}

	GP_FilterBrightness(*c, *c, filter_params, progress_callback);

	return 0;
}

/* contrast */

static struct param contrast_params[] = {
	{"mul", PARAM_FLOAT, "contrast (1.5 = +50%, 0.5 = -50%)", NULL, NULL},
	{"chann", PARAM_STR, "Channel name {R, G, B, A, V, ...}", NULL, NULL},
	{NULL,  0,           NULL,                    NULL, NULL}
};

static int contrast(GP_Context **c, const char *params)
{
	float mul = 0;
	char *chann = NULL;

	if (param_parse(params, contrast_params, "contrast", param_err, &mul, &chann))
		return EINVAL;

	if (mul <= 0) {
		print_error("contrast: mul parameter must be >= 0");
		return EINVAL;
	}
	
	GP_FILTER_PARAMS((*c)->pixel_type, filter_params);
	
	if (chann == NULL) {
		GP_FilterParamSetFloatAll(filter_params, mul);
	} else {
		GP_FilterParam *param = GP_FilterParamChannel(filter_params, chann);
	
		if (param == NULL) {
			print_error("contrast: Invalid channel name");
			return EINVAL;
		}

		GP_FilterParamSetFloatAll(filter_params, 1);
		param->val.f = mul;
	}

	GP_FilterContrast(*c, *c, filter_params, progress_callback);

	return 0;
}

/* noise */
static struct param noise_params[] = {
	{"ratio", PARAM_FLOAT, "noise", NULL, NULL},
	{NULL,  0,            NULL,    NULL, NULL}
};

static int noise(GP_Context **c, const char *params)
{
	float rat;

	if (param_parse(params, noise_params, "noise", param_err, &rat))
		return EINVAL;

	GP_FILTER_PARAMS((*c)->pixel_type, filter_params);
	
	GP_FilterParamSetFloatAll(filter_params, rat);

	GP_FilterNoise(*c, *c, filter_params, progress_callback);

	return 0;
}

/* invert */

static struct param invert_params[] = {
	{NULL,  0, NULL, NULL, NULL}
};

static int invert(GP_Context **c, const char *params)
{
	if (param_parse(params, invert_params, "invert", param_err))
		return EINVAL;

	GP_FilterInvert(*c, *c, progress_callback);

	return 0;
}

/* blur */

static struct param blur_params[] = {
	{"sigma", PARAM_FLOAT, "sigma parameter, radii of blur (sets both)", NULL, NULL},
	{"sigma_x", PARAM_FLOAT, "sigma parameter for horizontal direction", NULL, NULL},
	{"sigma_y", PARAM_FLOAT, "sigma parameter for vertical direction", NULL, NULL},
	{NULL,  0, NULL, NULL, NULL}
};

static int blur(GP_Context **c, const char *params)
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

	GP_FilterGaussianBlur(*c, *c, sigma_x, sigma_y, progress_callback);

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

static const GP_PixelType dither_pixel_types[] = {
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

static int dither(GP_Context **c, const char *params)
{
	int fmt = -1;

	if (param_parse(params, dither_params, "dither", param_err, &fmt))
		return EINVAL;
	
	if (fmt == -1) {
		print_error("dither: invalid format or format param missing");
		return EINVAL;
	}

	GP_Context *bw;
	bw = GP_FilterFloydSteinberg_RGB888_Alloc(*c, dither_pixel_types[fmt],
	                                          progress_callback);

	//TODO: so far we convert the context back to RGB888
	//(so we can do further work with it)
	GP_Blit(bw, 0, 0, GP_ContextW(bw), GP_ContextH(bw), *c, 0, 0);

	GP_ContextFree(bw);

	return 0;
}

/* jpg save filter */

static struct param save_jpg_params[] = {
	{"file", PARAM_STR, "Filename to save the result", NULL, NULL},
	{NULL,  0, NULL, NULL, NULL}
};

static int save_jpg(GP_Context **c, const char *params)
{
	char *file = NULL;

	if (param_parse(params, save_jpg_params, "jpg", param_err, &file))
		return EINVAL;
	
	if (file == NULL) {
		print_error("jpg: filename missing");
		return EINVAL;
	}

	GP_SaveJPG(*c, file, progress_callback);

	return 0;
}

/* png save filter */

static struct param save_png_params[] = {
	{"file", PARAM_STR, "Filename to save the result", NULL, NULL},
	{NULL,  0, NULL, NULL, NULL}
};

static int save_png(GP_Context **c, const char *params)
{
	char *file = NULL;

	if (param_parse(params, save_png_params, "png", param_err, &file))
		return EINVAL;
	
	if (file == NULL) {
		print_error("png: filename missing");
		return EINVAL;
	}

	GP_SavePNG(*c, file, progress_callback);

	return 0;
}

/* noise filter */

static struct param add_noise_params[] = {
	{"percents", PARAM_FLOAT, "Percents of noise to add", NULL, NULL},
	{"chann",    PARAM_STR,   "Channel name {R, G, B, A, V, ...}", NULL, NULL},
	{NULL,  0, NULL, NULL, NULL}
};

static uint32_t add_noise_op(uint32_t val, uint8_t bits, GP_FilterParam *param)
{
	float perc;
	int max = (1<<bits) - 1;
	int ret;

	perc = param->val.f;

	ret = val * (1 - perc) + (random() % max) * perc;

	if (ret < 0)
		ret = 0;

	if (ret > max)
		ret = max;

	return ret;
}

static uint32_t no_op(uint32_t val)
{
	return val;
}

static int add_noise(GP_Context **c, const char *params)
{
	float percents = 0;
	char *chann = NULL;

	if (param_parse(params, add_noise_params, "add_noise", param_err, &percents, &chann))
		return EINVAL;
	
	GP_FILTER_PARAMS((*c)->pixel_type, priv);
	GP_FilterParamSetFloatAll(priv, percents/100);
	GP_FILTER_PARAMS((*c)->pixel_type, op_callbacks);
	
	if (chann == NULL) {
		GP_FilterParamSetPtrAll(op_callbacks, add_noise_op);
	} else {
		GP_FilterParam *param = GP_FilterParamChannel(op_callbacks, chann);
	
		if (param == NULL) {
			print_error("add_noise: Invalid channel name");
			return EINVAL;
		}

		
		GP_FilterParamSetPtrAll(op_callbacks, no_op);
		param->val.ptr = add_noise_op;
	}

	GP_FilterPoint(*c, *c, op_callbacks, priv, progress_callback);

	return 0;
}

/* median filter */

static struct param median_params[] = {
	{"radius", PARAM_INT, "median radius for both x and y", NULL, NULL},
	{"radius_x", PARAM_INT, "median radius for x", NULL, NULL},
	{"radius_y", PARAM_INT, "median radius for y", NULL, NULL},
	{NULL,  0, NULL, NULL, NULL}
};

static int median(GP_Context **c, const char *params)
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

	GP_Context *ret = GP_FilterMedianAlloc(*c, rad_x, rad_y, progress_callback);

	if (ret == NULL)
		return ENOMEM;

	GP_ContextFree(*c);
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

static int sigma_mean(GP_Context **c, const char *params)
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

	(*c)->gamma = GP_GammaAcquire((*c)->pixel_type, 1.2);

	GP_Context *ret = GP_FilterSigmaAlloc(*c, rad_x, rad_y, min, sigma, progress_callback);

	if (ret == NULL)
		return ENOMEM;

	GP_ContextFree(*c);
	*c = ret;

	return 0;
}

/* laplacian edge sharpening filter */

static struct param sharpen_params[] = {
	{"weight", PARAM_FLOAT, "sharpening weight from [0,1] interval", NULL, NULL},
	{NULL,  0, NULL, NULL, NULL}
};

static int sharpen(GP_Context **c, const char *params)
{
	float weight = 0.1;

	if (param_parse(params, sharpen_params, "sigma", param_err, &weight))
		return EINVAL;
	
	GP_Context *ret = GP_FilterEdgeSharpeningAlloc(*c, weight, progress_callback);

	if (ret == NULL)
		return ENOMEM;

	GP_ContextFree(*c);
	*c = ret;

	return 0;
}

/* gaussian additive noise filter */

static struct param gauss_noise_params[] = {
	{"sigma", PARAM_FLOAT, "sigma: amount of noise between [0,1]", NULL, NULL},
	{"mu", PARAM_FLOAT, "mu: offset of noise between [0,1]", NULL, NULL},
	{NULL,  0, NULL, NULL, NULL}
};

static int gauss_noise(GP_Context **c, const char *params)
{
	float sigma = 0.1;
	float mu = 0;

	if (param_parse(params, gauss_noise_params, "gaussian noise", param_err, &sigma, &mu))
		return EINVAL;
	
	GP_FilterGaussianNoiseAdd(*c, *c, sigma, mu, progress_callback);

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

static int arithmetic(GP_Context **c, const char *params)
{
	char *file = NULL;
	int op = -1;

	if (param_parse(params, arithmetic_params, "arithmetic", param_err, &file, &op))
		return EINVAL;
	
	if (file == NULL) {
		print_error("arithmetic: Filename missing");
		return EINVAL;
	}

	GP_Context *img, *res = NULL;

	if ((img = GP_LoadImage(file, progress_callback)) == NULL) {
		print_error("arithmetic: Invalid image.");
		return EINVAL;
	}

	switch (op) {
	case 0:
		res = GP_FilterDifferenceAlloc(*c, img, progress_callback);
	break;
	case 1:
		res = GP_FilterAdditionAlloc(*c, img, progress_callback);
	break;
	case 2:
		res = GP_FilterMultiplyAlloc(*c, img, progress_callback);
	break;
	case 3:
		res = GP_FilterMinAlloc(*c, img, progress_callback);
	break;
	case 4:
		res = GP_FilterMaxAlloc(*c, img, progress_callback);
	break;
	}

	if (res == NULL)
		return ENOMEM;

	GP_ContextFree(*c);

	*c = res;

	return 0;
}

/* histogram */

static struct param histogram_params[] = {
	{"file", PARAM_STR, "Filename of image to use.", NULL, NULL},
	{NULL,  0, NULL, NULL, NULL}
};

static int histogram(GP_Context **c, const char *params)
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
	int (*apply)(GP_Context **c, const char *params);
};

static struct filter filter_table[] = {
	{"rotate",     "rotate image", rotate_params, rotate},
	{"mirror",     "mirror vertically/horizontally", mirror_params, mirror},
	{"scale",      "scale image to given width and height", scale_params,  scale},
	{"resize",     "resize image by given ratio", resize_params, resize},
	{"bright",     "alter image brightness", bright_params, bright},
	{"contrast",   "alter image contrast", contrast_params, contrast},
	{"invert",     "inverts image", invert_params, invert},
	{"add_noise",  "adds noise", add_noise_params, add_noise},
	{"noise",      "adds noise",  noise_params, noise},
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

static void apply_filters(GP_Context **src)
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

static void save_by_fmt(struct GP_Context *bitmap, const char *name, const char *fmt)
{
	int ret;

	progress_prefix = "Saving Image";

	if (!strcmp(fmt, "ppm"))
		ret = GP_SavePPM(bitmap, name, progress_callback);
	else if (!strcmp(fmt, "jpg"))
		ret = GP_SaveJPG(bitmap, name, progress_callback);
	else if (!strcmp(fmt, "png"))
		ret = GP_SavePNG(bitmap, name, progress_callback);
	
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
	GP_Context *bitmap;
	int opt, i;
	const char *out_fmt = "ppm";

	GP_ProgressCallback callback = {
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

			GP_SetDebugLevel(i);
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

		if ((bitmap = GP_LoadImage(argv[i], progress_callback)) == NULL) {
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
