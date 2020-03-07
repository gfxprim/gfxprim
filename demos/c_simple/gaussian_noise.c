// SPDX-License-Identifier: GPL-2.1-or-later
/*
 * Copyright (C) 2009-2013 Cyril Hrubis <metan@ucw.cz>
 */

 /*

   Gaussian additive noise example.

  */

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#include <gfxprim.h>

static void help(const char *app)
{
	printf("usage: %s -s float -m float input.img output.img\n\n", app);
	printf("     -s [0,1] sigma in pixels\n");
	printf("     -m [0,1] mu in pixels\n");
}

int main(int argc, char *argv[])
{
	gp_pixmap *img;
	float sigma = 0.1, mu = 0.1;
	int opt;

	while ((opt = getopt(argc, argv, "s:m:")) != -1) {
		switch (opt) {
		case 's':
			sigma = atof(optarg);
		break;
		case 'm':
			mu = atof(optarg);
		break;
		default:
			help(argv[0]);
			return 1;
		}
	}

	if (argc - optind != 2) {
		help(argv[0]);
		return 1;
	}

	img = gp_load_image(argv[optind], NULL);

	if (img == NULL) {
		fprintf(stderr, "Failed to load image '%s': %s\n",
		        argv[optind], strerror(errno));
		return 1;
	}

	gp_pixmap *res = gp_filter_gaussian_noise_add_alloc(img, sigma, mu, NULL);

	if (gp_save_image(res, argv[optind + 1], NULL)) {
		fprintf(stderr, "Failed to save image '%s': %s",
		        argv[optind + 1], strerror(errno));
		return 1;
	}

	return 0;
}
