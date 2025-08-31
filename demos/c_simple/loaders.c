// SPDX-License-Identifier: GPL-2.1-or-later
/*
 * Copyright (C) 2009-2012 Cyril Hrubis <metan@ucw.cz>
 */

 /*

   Simple loaders example utilizing progress callback.

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

	if (argc != 3) {
		fprintf(stderr, "usage: %s in_img out_img\n", argv[0]);
		return 1;
	}

	priv.op   = "Loading";
	priv.name = argv[1];

	img = gp_load_image(argv[1], &callback);

	if (img == NULL) {
		fprintf(stderr, "Failed to load image '%s':%s\n", argv[1],
		        strerror(errno));
		return 1;
	}

	printf("\n");


	priv.op   = "Saving";
	priv.name = argv[2];

	if (gp_save_image(img, argv[2], &callback)) {
		fprintf(stderr, "Failed to save image %s", strerror(errno));
		return 1;
	}

	printf("\n");

	return 0;
}
