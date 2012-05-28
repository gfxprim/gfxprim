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

   Read png meta-data and print them into stdout.

  */

#include <stdio.h>
#include <string.h>
#include <errno.h>

#include <GP.h>

#define SEP \
"-----------------------------------------------------------------------------"

int main(int argc, char *argv[])
{
	GP_MetaData *data = GP_MetaDataCreate(20);
	int i;

	if (argc < 2) {
		fprintf(stderr, "Takes an image(s) as parameter(s)\n");
		return 1;
	}
	
	//GP_SetDebugLevel(10);

	for (i = 1; i < argc; i++) {
		puts(SEP);
		printf("Opening '%s'\n", argv[i]);
		
		GP_MetaDataClear(data);
		
		if (GP_LoadPNGMetaData(argv[i], data)) {
			fprintf(stderr, "Failed to read '%s' meta-data: %s\n",
			        argv[1], strerror(errno));
		} else {
			GP_MetaDataPrint(data);
		}
	}
	
	puts(SEP);

	return 0;
}
