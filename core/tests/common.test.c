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

#include <stdlib.h>
#include <check.h>
#include <GP_Common.h>

/*
 * Demo ("testing" ;-) tests
 */

START_TEST(min_max)
{
	fail_unless(GP_MIN(-1.5, 2) == -1.5);
	fail_unless(GP_MAX(4294967295ULL, 1LL) == 4294967295ULL);
	int x=0, y=0;
	fail_unless(GP_MAX(x++, ++y) == 1);
	fail_unless(x == 1 && y == 1);
}
END_TEST

START_TEST(get_bits)
{
	fail_unless(GP_GET_BITS(15, 7, 0x12345678ULL) == 0x68);
	fail_unless(GP_GET_BITS(0, 0, 0x12345678ULL) == 0);
	fail_unless(GP_GET_BITS(16, 16, 0x1234) == 0);
	fail_unless(GP_GET_BITS(1, 32, 0x12345678ULL) == 0x091A2B3CULL);
}
END_TEST

START_TEST(set_bits)
{
	uint32_t x = 0x89ABC;
	uint16_t *y = &x; 
	GP_CLEAR_BITS(3, 4, x);
	fail_unless(x == 0x89A84);
	GP_SET_BITS_OR(10, x, 0x0000000); 
	fail_unless(x == 0x89A84);
	GP_SET_BITS(24, 18, x, 0x42F1); 
	fail_unless(x == 0xF1089A84);
	/* Check that only uint16_t is affected */
	GP_SET_BITS(0, 24, *y, 0x100F000); 
	fail_unless(x == 0xF108F000);
}
END_TEST

Suite *TS_Common(void)
{
	Suite *s = suite_create("core-common");

	TCase *tc = tcase_create("Common");
	tcase_add_test(tc, min_max);
	tcase_add_test(tc, get_bits);
	tcase_add_test(tc, set_bits);
	suite_add_tcase(s, tc);

	return s;
}
