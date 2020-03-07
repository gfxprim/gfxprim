// SPDX-License-Identifier: GPL-2.1-or-later
/*
 * Copyright (C) 2009-2012 Cyril Hrubis <metan@ucw.cz>
 */

 /*

   Convolution filter example.

  */

#include <stdio.h>
#include <string.h>
#include <errno.h>

#include <gfxprim.h>

struct callback_priv {
	char *op;
	char *name;
};

static int progress_callback(gp_progress_cb *self)
{
	struct callback_priv *priv = self->priv;

	printf("\r%s '%s' %3.1f%%", priv->op, priv->name, self->percentage);
	fflush(stdout);

	/*
	 * It's important to return zero as non-zero return value
	 * aborts the operation.
	 */
	return 0;
}

int main(int argc, char *argv[])
{
	gp_pixmap *img;
	struct callback_priv priv;
	gp_progress_cb callback = {.callback = progress_callback,
	                                .priv = &priv};

	if (argc != 2) {
		fprintf(stderr, "Takes an image as an parameter\n");
		return 1;
	}

	priv.op   = "Loading";
	priv.name = argv[1];

	img = gp_load_image(argv[1], &callback);

	if (img == NULL) {
		fprintf(stderr, "Failed to load image '%s': %s\n", argv[1],
		        strerror(errno));
		return 1;
	}

	printf("\n");

	unsigned int circle[] = {
		0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0,
		0, 0, 1, 1, 1, 1, 1, 1, 1, 0, 0,
		0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0,
		0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0,
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
		0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0,
		0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0,
		0, 0, 1, 1, 1, 1, 1, 1, 1, 0, 0,
		0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0,
	};

	unsigned int calpen[] = {
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	};

	unsigned int cross[] = {
		1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
		0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0,
		0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0,
		0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0,
		0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0,
		0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0,
		0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0,
		0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0,
		1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
	};

	unsigned int crazy[] = {
		0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0,
		1, 1, 1, 0, 0, 0, 0, 0, 1, 1, 1,
		1, 1, 0, 0, 0, 0, 0, 0, 0, 1, 1,
		1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
		1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
		1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
		1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
		1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
		1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
		1, 1, 0, 0, 0, 0, 0, 0, 0, 1, 1,
		1, 1, 1, 0, 0, 0, 0, 0, 1, 1, 1,
		0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0,
	};

	gp_median_weights weights = {
		.w = 11,
		.h = 11,
		.weights = circle,
	};

	priv.op = "Weighted Median";

	gp_pixmap *res = gp_filter_weighted_median_alloc(img, &weights, &callback);

	printf("\n");

	priv.op   = "Saving";
	priv.name = "out.png";

	if (gp_save_png(res, "out.png", &callback)) {
		fprintf(stderr, "Failed to save image: %s", strerror(errno));
		return 1;
	}

	printf("\n");

	return 0;
}
