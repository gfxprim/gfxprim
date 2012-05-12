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

   Simple loaders example.

  */

#include <stdio.h>
#include <string.h>
#include <errno.h>

#include <GP.h>

int main(int argc, char *argv[])
{
	GP_Context *img;

	/* Turn on debug messages */
	GP_SetDebugLevel(10);

	if (argc != 2) {
		fprintf(stderr, "Takes an image as an parameter\n");
		return 1;
	}
	
	img = GP_LoadImage(argv[1], NULL);

	if (img == NULL) {
		fprintf(stderr, "Failed to load image '%s':%s\n", argv[1],
		        strerror(errno));
		return 1;
	}

	if (GP_SavePNG(img, "out.png", NULL)) {
		fprintf(stderr, "Failed to save image %s", strerror(errno));
		return 1;
	}

	return 0;
}
