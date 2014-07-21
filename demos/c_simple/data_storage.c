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
 * Copyright (C) 2009-2014 Cyril Hrubis <metan@ucw.cz>                       *
 *                                                                           *
 *****************************************************************************/

 /*

   Data storage operations example.

  */

#include <stdio.h>

#include <GP.h>

int main(void)
{
	GP_DataStorage *storage = GP_DataStorageCreate();
	GP_DataNode *flags;

	if (storage == NULL)
		return 1;

	printf("Empty storage -----------------------\n");
	GP_DataPrint(GP_DataStorageRoot(storage));
	printf("-------------------------------------\n\n");

	GP_DataNode data = {
		.type = GP_DATA_INT,
		.value.i = 300,
		.id = "dpi"
	};

	GP_DataStorageAdd(storage, NULL, &data);

	GP_DataStorageAddString(storage, NULL, "orientation", "top-down");

	printf("Flat storage ------------------------\n");
	GP_DataPrint(GP_DataStorageRoot(storage));
	printf("-------------------------------------\n\n");

	flags = GP_DataStorageAddDict(storage, NULL, "flags");

	data.type = GP_DATA_INT;
	data.id = "compression_level";
	data.value.i = 10;

	GP_DataStorageAdd(storage, flags, &data);

	printf("Recursive storage -------------------\n");
	GP_DataPrint(GP_DataStorageRoot(storage));
	printf("-------------------------------------\n\n");

	GP_DataPrint(GP_DataStorageGet(storage, NULL, "dpi"));
	GP_DataStorageDestroy(storage);

	return 0;
}
