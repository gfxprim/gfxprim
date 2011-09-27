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
 * Copyright (C) 2009-2011 Cyril Hrubis <metan@ucw.cz>                       *
 *                                                                           *
 *****************************************************************************/

#include <stdio.h>
#include <stdlib.h>

#include "GP.h"

int main(int argc, char *argv[])
{
	GP_Context *bitmap;
	GP_SetDebugLevel(10);
	GP_RetCode ret;
	GP_Size w, h;

	if (argc < 4) {
		fprintf(stderr, "Usage: image w h\n");
		return 1;
	}

	if ((ret = GP_LoadImage(argv[1], &bitmap))) {
		fprintf(stderr, "Failed to load bitmap: %s\n", GP_RetCodeName(ret));
		return 1;
	}

	w = atoi(argv[2]);
	h = atoi(argv[3]);

	bitmap = GP_Scale(bitmap, w, h);

	if ((ret = GP_SavePPM("out.ppm", bitmap, "b"))) {
		fprintf(stderr, "Failed to load bitmap: %s\n", GP_RetCodeName(ret));
		return 1;
	}

	return 0;
}
