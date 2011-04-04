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
 * Copyright (C) 2011 Tomas Gavenciak <gavento@ucw.cz>                       *
 *                                                                           *
 *****************************************************************************/

/*
 
  Font load/save tests.

 */

#include <stdlib.h>
#include <string.h>
#include <GP.h>

#include "GP_Tests.h"

#define FONT_FILE "test_font.tmp"

GP_SUITE(font)

GP_TEST(load_save)
{
	GP_Font *loaded;
	int dd_size, ld_size;
	
	fail_unless(GP_FontSave(&GP_default_console_font, FONT_FILE) == GP_ESUCCESS);
	fail_unless(GP_FontLoad(&loaded, FONT_FILE) == GP_ESUCCESS);
	
	dd_size = GP_GetFontDataSize(&GP_default_console_font);
	ld_size = GP_GetFontDataSize(loaded);
	
	fail_unless(dd_size == ld_size);
	
	fail_unless(memcmp(GP_default_console_font.data, loaded->data, dd_size) == 0);

	/* cleanup */
	fail_unless(unlink(FONT_FILE) == 0);
}
END_TEST

