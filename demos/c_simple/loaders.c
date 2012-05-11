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

   Simple loaders example utilizing progress callback.

  */

#include <stdio.h>
#include <string.h>
#include <errno.h>

#include <GP.h>

struct callback_priv {
	char *op;
	char *name;
};

static int progress_callback(GP_ProgressCallback *self)
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
	GP_Context *img;
	struct callback_priv priv;
	GP_ProgressCallback callback = {.callback = progress_callback,
	                                .priv = &priv};

	if (argc != 2) {
		fprintf(stderr, "Takes an image as an parameter\n");
		return 0;
	}

	priv.op   = "Loading";
	priv.name = argv[1];
	
	img = GP_LoadImage(argv[1], &callback);

	if (img == NULL) {
		fprintf(stderr, "Failed to load image '%s':%s\n", argv[1],
		        strerror(errno));
		return 1;
	}

	printf("\n");


	priv.op   = "Saving";
	priv.name = "out.png";

	if (GP_SavePNG("out.png", img, &callback)) {
		fprintf(stderr, "Failed to save image %s", strerror(errno));
		return 1;
	}

	printf("\n");

	return 0;
}
