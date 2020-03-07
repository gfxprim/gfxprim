// SPDX-License-Identifier: GPL-2.1-or-later
/*
 * Copyright (C) 2009-2014 Cyril Hrubis <metan@ucw.cz>
 */

 /*

   Shows how to register custom image loader/saver.

   Feed it with small image (cca 60x60 pixels) to produce ASCII art version.

  */

#include <stdio.h>
#include <string.h>
#include <errno.h>

#include <gfxprim.h>

/*
 * Saves 2 bpp grayscale image as ASCII Art
 */
static int write_data(const gp_pixmap *img, gp_io *io,
                      gp_progress_cb *callback)
{
	gp_io *bio;
	int err;

	if (img->pixel_type != GP_PIXEL_G4) {
		errno = ENOSYS;
		return 1;
	}

	/* Create buffered I/O */
	bio = gp_io_wbuffer(io, 0);

	if (!bio)
		return 1;

	unsigned int i, j;
	const char *const table[] = {" ", ".", ",", "-", "=", "#", "O", "$"};

	for (j = 0; j < img->h; j++) {
		for (i = 0; i < img->w; i++) {
			gp_pixel p = gp_getpixel_raw(img, i, j);
			const char *c = table[p>>1];

			err = gp_io_flush(bio, c, 1);
			if (err)
				return 1;
		}

		if (gp_io_flush(bio, "\n", 1))
			return 1;

		if (gp_progress_cb_report(callback, j, img->h, img->w)) {
			errno = ECANCELED;
			return 1;
		}
	}

	gp_io_close(bio);
	gp_progress_cb_done(callback);
	return 0;
}

static gp_pixel_type save_ptypes[] = {
	GP_PIXEL_G4,
	GP_PIXEL_UNKNOWN,
};

const gp_loader loader = {
	.Write = write_data,
	.save_ptypes = save_ptypes,
	.fmt_name = "ASCII Art",
	.extensions = {"txt", NULL},
};

int main(int argc, char *argv[])
{
	gp_pixmap *c, *gc, *sc;

	gp_loader_register(&loader);

	/* List all loaders */
	gp_loaders_lists();
	printf("\n\n");

	if (argc != 2) {
		fprintf(stderr, "ERROR: Takes image as an argument\n");
		return 1;
	}

	/* Now load image and save it using our loader */
	c = gp_load_image(argv[1], NULL);
	if (c == NULL) {
		fprintf(stderr, "Failed to load image: %s\n", strerror(errno));
		return 1;
	}

	/*
	 * Font letters are not square resize the image so that it's twice it
	 * original width.
	 */
	gp_size w = 120;
	gp_size h = (w/2 * c->h + c->w/2)/c->w;

	sc = gp_filter_resize_alloc(c, w, h, GP_INTERP_LINEAR_LF_INT, NULL);
	gc = gp_filter_floyd_steinberg_alloc(sc, GP_PIXEL_G4, NULL);

	if (gc == NULL) {
		fprintf(stderr, "FloydSteinberg: %s\n", strerror(errno));
		return 1;
	}

	printf("Saving to test.txt\n");

	if (gp_save_image(gc, "test.txt", NULL)) {
		fprintf(stderr, "Failed to save image: %s\n", strerror(errno));
		return 1;
	}

	return 0;
}
