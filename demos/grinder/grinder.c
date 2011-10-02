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

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "GP.h"

#include "params.h"

static GP_ProgressCallback *progress_callback = NULL;

static const char *progress_prefix = NULL;

static void show_progress(GP_ProgressCallback *self)
{
	fprintf(stderr, "\rFilter %s %3.2f%%",
	        progress_prefix, self->percentage);
}

static int param_err(const struct param *self, const char *val, void *priv)
{
	/* invalid parameter name */
	if (self == NULL) {
		fprintf(stderr, "ERROR: %s: invalid parameter '%s'\n",
		                (char*)priv, val);
		return 1;
	}
	
	/* just regular error */
	fprintf(stderr, "ERROR: %s: invalid %s parameter %s = '%s'",
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
	"cubic",
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

static GP_RetCode resize(GP_Context **c, const char *params)
{
	int alg = 1;
	float ratio = -1;

	if (param_parse(params, resize_params, "resize", param_err,
	                &alg, &ratio))
		return GP_EINVAL;

	if (ratio == -1) {
		print_error("resize: ratio parameter is missing");
		return GP_EINVAL;
	}

	GP_Size w = ratio * (*c)->w;
	GP_Size h = ratio * (*c)->h;
	GP_Context *res = NULL;

	res = GP_FilterResize(*c, progress_callback, alg, w, h);
	
	if (res == NULL)
		return GP_EINVAL;

	GP_ContextFree(*c);
	*c = res;

	return GP_ESUCCESS;
}

/* scale filter */

static const char *scale_algs[] = {
	"nn",
	"cubic",
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

static GP_RetCode scale(GP_Context **c, const char *params)
{
	int alg = 1;
	int w = -1;
	int h = -1;

	if (param_parse(params, scale_params, "scale", param_err,
	                &alg, &w, &h))
		return GP_EINVAL;

	if (w == -1 && h == -1) {
		print_error("scale: w and/or h missing");
		return GP_EINVAL;
	}

	if (w == -1)
		w = (*c)->w * (1.00 * h/(*c)->h) + 0.5;
	
	if (h == -1)
		h = (*c)->h * (1.00 * w/(*c)->w) + 0.5;

	GP_Context *res = NULL;

	res = GP_FilterResize(*c, progress_callback, alg, w, h);

	if (res == NULL)
		return GP_EINVAL;

	GP_ContextFree(*c);
	*c = res;

	return GP_ESUCCESS;
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

static GP_RetCode rotate(GP_Context **c, const char *params)
{
	int rot = -1;

	if (param_parse(params, rotate_params, "rotate", param_err, &rot))
		return GP_EINVAL;

	if (rot == -1) {
		print_error("rotate: rot parameter is missing");
		return GP_EINVAL;
	}

	switch (rot) {
	case 0:
		GP_RotateCW(*c);
	break;
	case 1:
		GP_MirrorV(*c);
		GP_MirrorH(*c);
	break;
	case 2:
		GP_RotateCCW(*c);
	break;
	}

	return GP_ESUCCESS;
}

/* brightness filter */

static struct param bright_params[] = {
	{"inc", PARAM_INT, "brightness increment", NULL, NULL},
	{NULL,  0,         NULL,                   NULL, NULL}
};

static GP_RetCode bright(GP_Context **c, const char *params)
{
	int bright = 0;

	if (param_parse(params, bright_params, "bright", param_err, &bright))
		return GP_EINVAL;

	if (bright == 0) {
		print_error("bright: bright parameter is zero or missing");
		return GP_EINVAL;
	}

	GP_FilterBrightness_Raw(*c, *c, bright);

	return GP_ESUCCESS;
}

/* contrast */

static struct param contrast_params[] = {
	{"mul", PARAM_FLOAT, "contrast (1.5 = +50%, 0.5 = -50%)", NULL, NULL},
	{NULL,  0,           NULL,                    NULL, NULL}
};

static GP_RetCode contrast(GP_Context **c, const char *params)
{
	float mul = 0;

	if (param_parse(params, contrast_params, "contrast", param_err, &mul))
		return GP_EINVAL;

	if (mul <= 0) {
		print_error("contrast: mul parameter must be >= 0");
		return GP_EINVAL;
	}

	GP_FilterContrast_Raw(*c, *c, mul);

	return GP_ESUCCESS;
}

/* invert */

static struct param invert_params[] = {
	{NULL,  0, NULL, NULL, NULL}
};

static GP_RetCode invert(GP_Context **c, const char *params)
{
	if (param_parse(params, invert_params, "invert", param_err))
		return GP_EINVAL;

	GP_FilterInvert_Raw(*c, *c);

	return GP_ESUCCESS;
}

static struct param blur_params[] = {
	{"sigma", PARAM_FLOAT, "sigma parameter, radii of blur (sets both)", NULL, NULL},
	{"sigma_x", PARAM_FLOAT, "sigma parameter for horizontal direction", NULL, NULL},
	{"sigma_y", PARAM_FLOAT, "sigma parameter for vertical direction", NULL, NULL},
	{NULL,  0, NULL, NULL, NULL}
};

static GP_RetCode blur(GP_Context **c, const char *params)
{
	float sigma = 0;
	float sigma_x = 0;
	float sigma_y = 0;

	if (param_parse(params, blur_params, "blur", param_err, &sigma, &sigma_x, &sigma_y))
		return GP_EINVAL;

	if (sigma > 0) {
		sigma_x = sigma;
		sigma_y = sigma;
	}

	if (sigma_x <= 0 || sigma_y <= 0) {
		print_error("blur: sigma_x and sigma_y parameter must be >= 0");
		return GP_EINVAL;
	}

	GP_FilterGaussianBlur_Raw(*c, *c, progress_callback, sigma_x, sigma_y);

	return GP_ESUCCESS;
}

/* filters */

struct filter {
	const char *name;
	const char *desc;
	struct param *param_desc;
	GP_RetCode (*apply)(GP_Context **c, const char *params);
};

static struct filter filter_table[] = {
	{"resize",   "resize image by given ratio", resize_params, resize},
	{"scale",    "scale image to given width and height", scale_params,  scale},
	{"rotate",   "rotate image", rotate_params, rotate},
	{"bright",   "alter image brightness", bright_params, bright},
	{"contrast", "alter image contrast", contrast_params, contrast},
	{"invert",   "inverts image", invert_params, invert},
	{"blur",     "gaussian blur", blur_params, blur},
	{NULL, NULL, NULL, NULL}
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
	GP_RetCode ret;

	for (i = 0; i < filter_cnt; i++) {
		
		progress_prefix = filters[i]->name;

		if ((ret = filters[i]->apply(src, filter_params[i]))) {
			fprintf(stderr, "Error: %s\n", GP_RetCodeName(ret));
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
	"                +-+-----+                             \n"
	"               /  |  +-+| .11.                        \n"
	"            +-{  D|  |010101011.                      \n"
	"            |  \\  |  +-.0100101.                      \n"
	"          O=+   +-+-----+ .10110101.                  \n"
	"                            .010101.                  \n"
	"                              .1.                     \n"
	"                                                      \n"
	"                 Program options                      \n"
	"                 ===============                      \n"
	"                                                      \n"
	"-h        - prints this help                          \n"
	"-p        - show filter progress                      \n"
	"-v int    - sets gfxprim verbosity level              \n"
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

int main(int argc, char *argv[])
{
	GP_Context *bitmap;
	GP_RetCode ret;
	int opt, i;

	GP_ProgressCallback callback = {
		.callback = show_progress,
	};

	while ((opt = getopt(argc, argv, "f:hpv:")) != -1) {
		switch (opt) {
		case 'h':
			print_help();
			return 0;
		break;
		case 'v':
			GP_SetDebugLevel(atoi(optarg));
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
		fprintf(stderr, "Expected bitmap filenames\n");
		print_help();
		return 1;
	}
	
	for (i = optind; i < argc; i++) {
		char buf[255];
		
		snprintf(buf, sizeof(buf), "out_%i.ppm", i - optind + 1);
		fprintf(stderr, "Processing '%s' -> '%s'\n", argv[i], buf);

		if ((ret = GP_LoadImage(argv[i], &bitmap))) {
			fprintf(stderr, "Failed to load bitmap: %s\n",
			                GP_RetCodeName(ret));
			return 1;
		}

		apply_filters(&bitmap);


		if ((ret = GP_SavePPM(buf, bitmap, "b"))) {
			fprintf(stderr, "Failed to load bitmap: %s\n", GP_RetCodeName(ret));
			return 1;
		}
		
	}
	
	return 0;
}
