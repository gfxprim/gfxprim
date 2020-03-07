// SPDX-License-Identifier: GPL-2.1-or-later
/*
 * Copyright (C) 2009-2012 Cyril Hrubis <metan@ucw.cz>
 */

 /*

   Simple loaders example.

  */

#include <stdio.h>
#include <string.h>
#include <errno.h>

#include <gfxprim.h>

int main(int argc, char *argv[])
{
	gp_pixmap *img;

	/* Turn on debug messages */
	gp_set_debug_level(10);

	if (argc != 2) {
		fprintf(stderr, "Takes an image as an parameter\n");
		return 1;
	}

	img = gp_load_image(argv[1], NULL);

	if (img == NULL) {
		fprintf(stderr, "Failed to load image '%s':%s\n", argv[1],
		        strerror(errno));
		return 1;
	}

	if (gp_save_png(img, "out.png", NULL)) {
		fprintf(stderr, "Failed to save image %s", strerror(errno));
		return 1;
	}

	return 0;
}
