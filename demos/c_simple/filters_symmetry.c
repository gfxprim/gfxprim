// SPDX-License-Identifier: GPL-2.1-or-later
/*
 * Copyright (C) 2009-2012 Cyril Hrubis <metan@ucw.cz>
 */

 /*

   Symmetry filter example.

  */

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <getopt.h>

#include <gfxprim.h>

static void usage_and_exit(int ret)
{
	int i;
	printf("filter_symmetry [-d debug_level] -s {");

	for (i = 0; gp_filter_symmetry_names[i+1] != NULL; i++)
		printf("%s, ", gp_filter_symmetry_names[i]);

	printf("%s} image_in image_out\n", gp_filter_symmetry_names[i]);

	exit(ret);
}

int main(int argc, char *argv[])
{
	gp_pixmap *src, *res;
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
	gp_set_debug_level(debug);

	if (symmetry == NULL) {
		printf("Symmetry not specified\n");
		usage_and_exit(1);
	}

	if (argc - optind != 2) {
		printf("Input and output image not specified\n");
		usage_and_exit(1);
	}

	sym = gp_filter_symmetry_by_name(symmetry);

	if (sym < 0) {
		printf("Invalid symmetry name '%s'\n", symmetry);
		usage_and_exit(1);
	}

	/* Load Image  */
	src = gp_load_image(argv[optind], NULL);

	if (src == NULL) {
		fprintf(stderr, "Failed to load image '%s': %s\n",
		        argv[optind], strerror(errno));
		return 1;
	}

	/* Apply a symmetry filter */
	res = gp_filter_symmetry_alloc(src, sym, NULL);

	/* Save Image */
	if (gp_save_image(res, argv[optind+1], NULL)) {
		fprintf(stderr, "Failed to save image '%s': %s\n",
		        argv[optind+1], strerror(errno));
		return 1;
	}

	/* Cleanup */
	gp_pixmap_free(src);
	gp_pixmap_free(res);

	return 0;
}
