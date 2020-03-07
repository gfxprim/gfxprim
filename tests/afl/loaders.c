// SPDX-License-Identifier: GPL-2.1-or-later
/*
 * Copyright (C) 2009-2015 Cyril Hrubis <metan@ucw.cz>
 */

 /*

   Minimal loaders call to be used by american fuzzy lop (afl).

  */

#include <stdio.h>
#include <string.h>
#include <errno.h>

#include <gfxprim.h>

int main(int argc, char *argv[])
{
	gp_pixmap *img;

	if (argc != 2) {
		fprintf(stderr, "Takes an image as an parameter\n");
		return 1;
	}

	img = gp_load_image(argv[1], NULL);

	gp_pixmap_free(img);

	return 0;
}
