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

   Shows how to register custom image loader/saver.

   Feed it with small image (cca 60x60 pixels) to produce ascii art version.

  */

#include <stdio.h>
#include <string.h>
#include <errno.h>

#include <GP.h>

/*
 * Saves 2 bpp grayscale image as ASCII Art
 */
static int save(const GP_Context *img, const char *dst_path,
                GP_ProgressCallback *callback)
{
	if (img->pixel_type != GP_PIXEL_G2) {
		errno = ENOSYS;
		return 1;
	}

	FILE *f = fopen(dst_path, "w");

	if (f == NULL)
		return 1;

	unsigned int i, j;

	for (j = 0; j < img->h; j++) {
		for (i = 0; i < img->w; i++) {
			GP_Pixel p = GP_GetPixel_Raw(img, i, j);

			switch (p) {
			case 0:
				fprintf(f, "  ");
			break;
			case 1:
				fprintf(f, "..");
			break;
			case 2:
				fprintf(f, "()");
			break;
			case 3:
				fprintf(f, "OO");
			break;
			}
		}

		fprintf(f, "\n");

		if (GP_ProgressCallbackReport(callback, img->h, j, img->w)) {
			fclose(f);
			unlink(dst_path);
			errno = ECANCELED;
			return 1;
		}
	}

	if (fclose(f))
		return 1;

	GP_ProgressCallbackDone(callback);

	return 0;
}

GP_Loader loader = {
	.Load = NULL,
	.Save = save,
	.Match = NULL,
	.fmt_name = "ASCII Art",
	.extensions = {"txt", NULL},
};

int main(int argc, char *argv[])
{
	GP_Context *c, *gc;

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

	gc = GP_FilterFloydSteinberg_RGB888_Alloc(c, GP_PIXEL_G2, NULL);

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
