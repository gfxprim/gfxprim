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

#include "GP_Tests.h"

#include <GP_Common.h>

/*
 * Demo ("testing" ;-) tests for GP_Common.h
 */

GP_SUITE(GP_Common) 

GP_TEST(min_max)
{
	fail_unless(GP_MIN(-1.5, 2) == -1.5);
	fail_unless(GP_MAX(4294967295ULL, 1ULL) == 4294967295ULL);
	int x=0, y=0;
	fail_unless(GP_MAX(x++, ++y) == 1);
	fail_unless(x == 1 && y == 1);
}
GP_ENDTEST

GP_TEST(get_bits)
{
	fail_unless(GP_GET_BITS(15, 7, 0x12345678ULL) == 0x68);
	fail_unless(GP_GET_BITS(0, 0, 0x12345678ULL) == 0);
	fail_unless(GP_GET_BITS(16, 16, 0x1234) == 0);
	fail_unless(GP_GET_BITS(1, 32, 0x12345678ULL) == 0x091A2B3CULL);
}
GP_ENDTEST

GP_TEST(set_bits)
{
	uint32_t x = 0x89ABC;
	uint16_t *y = (uint16_t*) &x; 
	GP_CLEAR_BITS(3, 4, x);
	fail_unless(x == 0x89A84);
	GP_SET_BITS_OR(10, x, 0x0000000); 
	fail_unless(x == 0x89A84);
	GP_SET_BITS(24, 18, x, 0x42F1); 
	fail_unless(x == 0xF1089A84);
	/* Check that only uint16_t is affected */
	GP_SET_BITS(0, 24, *y, 0x100F000LL); 
	fail_unless(x == 0xF108F000);
}
GP_ENDTEST

GP_TEST(abort_check_assert, "loop_start=0, loop_end=9, expect_exit=1")
{
	if (_i==0) GP_ABORT();
	if (_i==1) GP_ABORT("MSG");
	if (_i==2) GP_ABORT("FORMAT %d", _i);
	if (_i==3) GP_ASSERT(1==0);
	if (_i==4) GP_ASSERT(1==0, "MSG");
	if (_i==5) GP_ASSERT(1==0, "FORMAT %d", _i);
	if (_i==6) GP_CHECK(1==0);
	if (_i==7) GP_CHECK(1==0, "MSG");
	if (_i==8) GP_CHECK(1==0, "FORMAT %d", _i);
}
END_TEST

GP_TEST(assert_check_nop)
{
	GP_ASSERT(1);
	GP_ASSERT(1, "MSG");
	GP_CHECK(1);
	GP_CHECK(1, "MSG");
}
GP_ENDTEST
