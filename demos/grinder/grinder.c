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

static int param_err(const struct param *self, const char *val, void *priv)
{
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
                              void *val)
{
	float f = *((float*)val);

	if (f <= 0)
		return -1;
	
	return 0;
}

static struct param resize_params[] = {
	{"alg",   PARAM_ENUM,  resize_algs, NULL},
	{"ratio", PARAM_FLOAT, NULL,        resize_check_ratio},
	{NULL,    0,           NULL,        NULL}
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

	switch (alg) {
	case 0:
		res = GP_Scale_NN(*c, w, h);
	break;
	case 1:
		res = GP_Scale_BiCubic(*c, w, h);
	break;
	}

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
                            void *val)
{
	int i = *((int*)val);

	if (i <= 0)
		return 1;
	
	return 0;
}

static struct param scale_params[] = {
	{"alg", PARAM_ENUM,  scale_algs, NULL},
	{"w",   PARAM_INT,   NULL,       scale_check_size},
	{"h",   PARAM_INT,   NULL,       scale_check_size},
	{NULL,  0,           NULL,       NULL}
};

static GP_RetCode scale(GP_Context **c, const char *params)
{
	int alg = 1;
	int w = -1;
	int h = -1;

	if (param_parse(params, scale_params, "scale", param_err,
	                &alg, &w, &h))
		return GP_EINVAL;

	if (w == -1 || h == -1) {
		print_error("scale: w and/or h missing");
		return GP_EINVAL;
	}

	GP_Context *res = NULL;

	switch (alg) {
	case 0:
		res = GP_Scale_NN(*c, w, h);
	break;
	case 1:
		res = GP_Scale_BiCubic(*c, w, h);
	break;
	}

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
	{"rot", PARAM_ENUM,  rotate_rots, NULL},
	{NULL,     0,           NULL,       NULL}
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
	{"val", PARAM_INT, NULL, NULL},
	{NULL,  0,         NULL, NULL}
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

	GP_Context *res = GP_FilterBrightness(*c, bright);

	if (res == NULL)
		return GP_EINVAL;

	GP_ContextFree(*c);
	*c = res;
	
	return GP_ESUCCESS;
}

/* filters */

struct filter {
	char *name;
	char *param_help;
	GP_RetCode (*apply)(GP_Context **c, const char *params);
};

static struct filter filter_table[] = {
	{"resize", "alg=nn|cubic:ratio=float", resize},
	{"scale",  "alg=nn|cubic:w=int:h=int", scale},
	{"rotate", "rot=90|180|270",           rotate},
	{"bright", "val=int",                  bright},
	{NULL, NULL, NULL}
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

static void print_filter_help(const char *prefix)
{
	unsigned int i;

	for (i = 0; filter_table[i].name != NULL; i++) {
		printf("%s%s : %s\n", prefix, filter_table[i].name,
		       filter_table[i].param_help);
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

	for (i = 0; i < filter_cnt; i++)
		if ((ret = filters[i]->apply(src, filter_params[i]))) {
			fprintf(stderr, "Error: %s\n", GP_RetCodeName(ret));
			exit(1);
		}
}

static const char *app_help = {
	"              <-  Bitmap Grinder  ->                  \n"
	"                                                      \n"
	"-h        : prints this help                          \n"
	"-v int    : sets gfxprim verbosity level              \n"
	"-f params : apply filter, multiple filters may be used\n"
	"                                                      \n"
	"                 List of filters                      \n"
};

static void print_help(void)
{
	puts(app_help);
	print_filter_help("  ");
}

int main(int argc, char *argv[])
{
	GP_Context *bitmap;
	GP_RetCode ret;
	int opt, i;

	while ((opt = getopt(argc, argv, "f:hv:")) != -1) {
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
