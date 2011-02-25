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

  */

#include <GP.h>
#include <GP_PGM.h>

#define W 221
#define H 160

int main(void)
{
	int i;

	GP_Context *context = GP_ContextAlloc(W, H, GP_PIXEL_G2);

	if (context == NULL) {
		fprintf(stderr, "Couldn't allocate context\n");
		return 1;
	}

	for (i = 0; i < 20; i++)
		GP_HLineXYW(context, 0, i, i, 3);
	
	for (i = 0; i < 20; i++)
		GP_HLineXYW(context, 1, i + 20, i, 2);
	
	for (i = 0; i < 20; i++)
		GP_HLineXYW(context, 2, i + 40, i, 1);
	
	for (i = 0; i < 20; i++)
		GP_HLineXYW(context, 3, i + 60, i, 3);
	
	for (i = 0; i < 20; i++)
		GP_HLineXYW(context, 4, i + 80, i, 2);

	GP_Line(context, 0, 0, W, H, 3);
	GP_Line(context, 0, H, W, 0, 3);

	GP_FillCircle(context, W/2, H/2, 19, 3);
	GP_FillCircle(context, W/2, H/2, 7, 2);
	GP_FillCircle(context, W/2, H/2, 5, 1);
	GP_FillCircle(context, W/2, H/2, 2, 0);
	GP_Text(context, NULL, 60, 10, GP_VALIGN_BELOW | GP_ALIGN_RIGHT, "Test", 3);
	GP_Text(context, NULL, 60, 20, GP_VALIGN_BELOW | GP_ALIGN_RIGHT, "Test", 2);
	GP_Text(context, NULL, 60, 30, GP_VALIGN_BELOW | GP_ALIGN_RIGHT, "Test", 1);

	if (GP_SavePGM("test.pgm", context)) {
		fprintf(stderr, "Can't save context\n");
		return 1;
	}

	GP_ContextFree(context);

	return 0;
}
