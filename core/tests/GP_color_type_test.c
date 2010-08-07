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

 /*

   Ensure that we got the color we asked for.

  */

#include <GP.h>

int main(void)
{
	GP_ColorType i, j;
	GP_Color col;
	GP_RetCode ret;

	for (i = 0; i < GP_COLMAX; i++)
		for (j = 0; j < GP_COLMAX; j++) {
			col.type = i;
			ret = GP_ColorConvert(&col, j);

			printf("%s -> %s: %s ...... ", GP_ColorTypeName(i),
			                               GP_ColorTypeName(j),
					               GP_RetCodeName(ret));

			if (ret == GP_ESUCCESS || ret == GP_EUNPRECISE) {
				if (col.type != j) {
					printf("got %s ERROR\n",
					       GP_ColorTypeName(col.type));
					continue;
				}
			}

			printf("OK\n");
		}

	return 0;
}
