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
 * Copyright (C) 2009-2014 Cyril Hrubis <metan@ucw.cz>                       *
 *                                                                           *
 *****************************************************************************/

 /*

   Shows how to register custom image loader/saver.

   Feed it with small image (cca 60x60 pixels) to produce ASCII art version.

  */

#include <stdio.h>
#include <string.h>
#include <errno.h>

#include <GP.h>

/*
 * Saves 2 bpp grayscale image as ASCII Art
 */
static int write_data(const GP_Pixmap *img, GP_IO *io,
                      GP_ProgressCallback *callback)
{
	GP_IO *bio;
	int err;

	if (img->pixel_type != GP_PIXEL_G2) {
		errno = ENOSYS;
		return 1;
	}

	/* Create buffered I/O */
	bio = GP_IOWBuffer(io, 0);

	if (!bio)
		return 1;

	unsigned int i, j;

	for (j = 0; j < img->h; j++) {
		for (i = 0; i < img->w; i++) {
			GP_Pixel p = GP_GetPixel_Raw(img, i, j);

			switch (p) {
			case 0:
				err = GP_IOFlush(bio, "  ", 2);
			break;
			case 1:
				err = GP_IOFlush(bio, "..", 2);
			break;
			case 2:
				err = GP_IOFlush(bio, "()", 2);
			break;
			case 3:
				err = GP_IOFlush(bio, "OO", 2);
			break;
			}

			if (err)
				return 1;
		}

		if (GP_IOFlush(bio, "\n", 1))
			return 1;

		if (GP_ProgressCallbackReport(callback, img->h, j, img->w)) {
			errno = ECANCELED;
			return 1;
		}
	}

	GP_ProgressCallbackDone(callback);
	return 0;
}

static GP_PixelType save_ptypes[] = {
	GP_PIXEL_G2,
	GP_PIXEL_UNKNOWN,
};

GP_Loader loader = {
	.Write = write_data,
	.save_ptypes = save_ptypes,
	.fmt_name = "ASCII Art",
	.extensions = {"txt", NULL},
};

int main(int argc, char *argv[])
{
	GP_Pixmap *c, *gc;

	GP_LoaderRegister(&loader);

	/* List all loaders */
	GP_ListLoaders();
	printf("\n\n");

	if (argc != 2) {
		fprintf(stderr, "ERROR: Takes image as an argument\n");
		return 1;
	}

	/* Now load image and save it using our loader */
	c = GP_LoadImage(argv[1], NULL);

	if (c == NULL) {
		fprintf(stderr, "Failed to load image: %s\n", strerror(errno));
		return 1;
	}

	gc = GP_FilterFloydSteinbergAlloc(c, GP_PIXEL_G2, NULL);

	if (gc == NULL) {
		fprintf(stderr, "FloydSteinberg: %s\n", strerror(errno));
		return 1;
	}

	printf("Saving to test.txt\n");

	if (GP_SaveImage(gc, "test.txt", NULL)) {
		fprintf(stderr, "Failed to save image: %s\n", strerror(errno));
		return 1;
	}

	return 0;
}
