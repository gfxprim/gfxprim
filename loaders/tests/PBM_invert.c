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
 * Copyright (C) 2009-2010 Jiri "BlueBear" Dluhos                            *
 *                         <jiri.bluebear.dluhos@gmail.com>                  *
 *                                                                           *
 * Copyright (C) 2009-2010 Cyril Hrubis <metan@ucw.cz>                       *
 *                                                                           *
 *****************************************************************************/

#include <GP.h>
#include <GP_PBM.h>

int main(int argc, char *argv[])
{
	uint64_t i;
	GP_Context *context;

	if (argc < 3) {
		fprintf(stderr, "Usage: %s input.pbm output.pbm\n", argv[0]);
		return 1;
	}

	printf("inverting %s -> %s\n", argv[1], argv[2]);

 	if (GP_LoadPBM(argv[1], &context)) {
		fprintf(stderr, "Couldn't load %s\n", argv[1]);
		return 1;
	}

	for (i = 0; i < context->h * context->bytes_per_row; i++)
		context->pixels[i] = ~context->pixels[i];

	if (GP_SavePBM(argv[2], context)) {
		fprintf(stderr, "Can't save %s\n", argv[2]);
		return 1;
	}

	GP_ContextFree(context);

	return 0;
}
