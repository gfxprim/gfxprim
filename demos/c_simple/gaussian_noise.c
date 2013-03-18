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

   Gaussian additive noise example.

  */

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#include <GP.h>

static void help(const char *app)
{
	printf("usage: %s -s float -m float input.img output.img\n\n", app);
	printf("     -s [0,1] sigma in pixels\n");
	printf("     -m [0,1] mu in pixels\n");
}

int main(int argc, char *argv[])
{
	GP_Context *img;
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

	img = GP_LoadImage(argv[optind], NULL);

	if (img == NULL) {
		fprintf(stderr, "Failed to load image '%s': %s\n",
		        argv[optind], strerror(errno));
		return 1;
	}

	GP_Context *res = GP_FilterGaussianNoiseAddAlloc(img, sigma, mu, NULL);

	if (GP_SaveImage(res, argv[optind + 1], NULL)) {
		fprintf(stderr, "Failed to save image '%s': %s",
		        argv[optind + 1], strerror(errno));
		return 1;
	}

	return 0;
}
