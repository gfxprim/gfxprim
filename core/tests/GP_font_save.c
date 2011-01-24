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

 /* Font saving test. */

#include <GP.h>
#include <stdio.h>

int main(void)
{
	GP_RetCode retcode;
	
	retcode = GP_FontSave(&GP_default_console_font, "test_font.tmp");
	if (retcode != GP_ESUCCESS) {
		fprintf(stderr, "Error trying to save a font: %s\n",
			GP_RetCodeName(retcode));
		return 1;
	}

	GP_Font *loaded;
	retcode = GP_FontLoad(&loaded, "test_font.tmp");
	if (retcode != GP_ESUCCESS) {
		fprintf(stderr, "Error trying to re-load a saved font: %s\n",
			GP_RetCodeName(retcode));
		return 1;
	}

	return 0;
}
