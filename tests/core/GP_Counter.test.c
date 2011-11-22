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
 * Copyright (C) 2011      Tomas Gavenciak <gavento@ucw.cz>                  *
 *                                                                           *
 *****************************************************************************/

#include <string.h>
#include <stdio.h>

#include "GP_Tests.h"
#include "GP_Counter.h"

GP_SUITE(GP_Counter)

GP_TEST(Smoke)
{
	fail_unless(GP_CounterVal(NULL) == 0);
	GP_IncCounter(NULL);
	GP_AddCounter(NULL, -10);
	fail_unless(GP_GetCounter("") != NULL);

	GP_IncCounter(GP_GetCounter("a"));
	fail_unless(GP_CounterVal(GP_GetCounter("a")) == 1);

	GP_PrintCounters(fopen("/dev/null","wt"));
}
END_TEST

GP_TEST(Allocation)
{
	/* random-like operations with counters,
	 * should test reasonably many combinations */
	GP_SetCounter(GP_GetCounter("a"), 11);
	GP_AddCounter(GP_GetCounter("e"), -42);
	GP_Counter b = GP_GetCounter("b");
	GP_IncCounter(GP_GetCounter("a"));
	GP_GetCounter("d");
	GP_AddCounter(GP_GetCounter("c"), 21);
	GP_IncCounter(GP_GetCounter("b"));
	GP_AddCounter(GP_GetCounter("b"), -8);
	GP_IncCounter(b);
	GP_SetCounter(GP_GetCounter("f"), 91);
	GP_SetCounter(GP_GetCounter("f"), -1);
	
	fail_unless(GP_CounterVal(GP_GetCounter("a")) == 12);
	fail_unless(GP_CounterVal(GP_GetCounter("b")) == -6);
	fail_unless(GP_CounterVal(GP_GetCounter("c")) == 21);
	fail_unless(GP_CounterVal(GP_GetCounter("d")) == 0);
	fail_unless(GP_CounterVal(GP_GetCounter("e")) == -42);
	fail_unless(GP_CounterVal(GP_GetCounter("f")) == -1);
}
END_TEST

GP_TEST(Overflow)
{
	char buf[8];
	int i;
	for (i = 0; i < GP_COUNTER_MAX; i++) {
		sprintf(buf, "%d", i);
		fail_if(GP_GetCounter(buf) == NULL);
	}
	fail_unless(GP_GetCounter("next") == NULL);
}
END_TEST

