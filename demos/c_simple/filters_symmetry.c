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

   Symmetry filter example.

  */

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <getopt.h>

#include <GP.h>

static void usage_and_exit(int ret)
{
	int i;
	printf("filter_symmetry [-d debug_level] -s {");

	for (i = 0; GP_FilterSymmetryNames[i+1] != NULL; i++)
		printf("%s, ", GP_FilterSymmetryNames[i]);

	printf("%s} image_in image_out\n", GP_FilterSymmetryNames[i]);

	exit(ret);
}

int main(int argc, char *argv[])
{
	GP_Pixmap *src, *res;
	const char *symmetry = NULL;
	int opt, sym, debug = 0;

	/* Parse program options */
	while ((opt = getopt(argc, argv, "d:hs:")) != -1) {
		switch (opt) {
		case 's':
			symmetry = optarg;
		break;
		case 'h':
			usage_and_exit(0);
		break;
		case 'd':
			debug = atoi(optarg);
		break;
		default:
			usage_and_exit(1);
		}
	}

	/* Turn on debug messages */
	GP_SetDebugLevel(debug);

	if (symmetry == NULL) {
		printf("Symmetry not specified\n");
		usage_and_exit(1);
	}

	if (argc - optind != 2) {
		printf("Input and output image not specified\n");
		usage_and_exit(1);
	}

	sym = GP_FilterSymmetryByName(symmetry);

	if (sym < 0) {
		printf("Invalid symmetry name '%s'\n", symmetry);
		usage_and_exit(1);
	}

	/* Load Image  */
	src = GP_LoadImage(argv[optind], NULL);

	if (src == NULL) {
		fprintf(stderr, "Failed to load image '%s': %s\n",
		        argv[optind], strerror(errno));
		return 1;
	}

	/* Apply a symmetry filter */
	res = GP_FilterSymmetryAlloc(src, sym, NULL);

	/* Save Image */
	if (GP_SaveImage(res, argv[optind+1], NULL)) {
		fprintf(stderr, "Failed to save image '%s': %s\n",
		        argv[optind+1], strerror(errno));
		return 1;
	}

	/* Cleanup */
	GP_PixmapFree(src);
	GP_PixmapFree(res);

	return 0;
}
