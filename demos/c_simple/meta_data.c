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

   Meta-data storage operations example.

   Meta-data storage is used to store image meta-data (if present) such as
   physical size, creation date, etc...

   Meta-data storage is basically an typed dictionary.

   This example shows low-level interface to GP_MetaData structure.

  */

#include <stdio.h>

#include <GP.h>

int main(void)
{
	GP_MetaData *data = GP_MetaDataCreate(10);

	//GP_SetDebugLevel(10);

	if (data == NULL)
		return 1;

	/*
	 * Create integer
	 *
	 * May fail, if there is allready record with id 'dpi' or
	 * if there is no space left.
	 */
	GP_MetaDataCreateInt(data, "dpi", 300);

	/*
	 * Create an string.
	 *
	 * The last parameter says, if the string should be duplicated
	 * in the metadata storage.
	 */
	GP_MetaDataCreateString(data, "author", "Foo Bar <foo@bar.net>", 0, 1);
	GP_MetaDataCreateString(data, "comment", "Created in hurry.", 0, 1);
	GP_MetaDataCreateDouble(data, "pi", 3.141592);

	const char *ret;

	ret = GP_MetaDataGetString(data, "comment");

	if (ret != NULL)
		printf("Found string 'comment' = '%s'\n", ret);
	else
		printf("ERROR: cannot cound string 'comment'\n");

	printf("\n");

	/*
	 * Print all meta-data
	 */
	GP_MetaDataPrint(data);

	return 0;
}
