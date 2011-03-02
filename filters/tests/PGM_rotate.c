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
#include <GP_PGM.h>
#include <GP_Rotate.h>

int main(int argc, char *argv[])
{
	GP_Context *context, *copy;

	if (argc < 2)
		fprintf(stderr, "Takes one parameter, filename\n");

	if (GP_LoadPGM(argv[1], &context)) {
		fprintf(stderr, "Can't load context\n");
		return 1;
	}

	printf("Loaded PGM %u bpp w %u h %u, bpr %u\n",
	       context->bpp, context->w, context->h, context->bytes_per_row);

	copy = GP_ContextCopy(context, GP_COPY_WITH_PIXELS);
	GP_MirrorH(copy);
	
	if (GP_SavePGM("test-mh.pgm", copy)) {
		fprintf(stderr, "Can't save context\n");
		return 1;
	}
	
	GP_ContextFree(copy);
	copy = GP_ContextCopy(context, GP_COPY_WITH_PIXELS);

	GP_MirrorV(copy);
	
	if (GP_SavePGM("test-mv.pgm", copy)) {
		fprintf(stderr, "Can't save context\n");
		return 1;
	}
	
	GP_ContextFree(copy);
	copy = GP_ContextCopy(context, GP_COPY_WITH_PIXELS);

	GP_RotateCW(copy);
	
	if (GP_SavePGM("test-cw.pgm", copy)) {
		fprintf(stderr, "Can't save context\n");
		return 1;
	}

	GP_ContextFree(copy);
	copy = GP_ContextCopy(context, GP_COPY_WITH_PIXELS);

	GP_RotateCCW(copy);
	
	if (GP_SavePGM("test-ccw.pgm", copy)) {
		fprintf(stderr, "Can't save context\n");
		return 1;
	}

	GP_ContextFree(copy);
	GP_ContextFree(context);

	return 0;
}
