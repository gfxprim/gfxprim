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

#include "GP_Test.h"
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
}

GP_TEST(Allocation)
{
  GP_IncCounter(GP_GetCounter("a"));
  GP_AddCounter(GP_GetCounter("e"), -42);
  GP_AddCounter(GP_GetCounter("b"), -8);
  GP_IncCounter(GP_GetCounter("a"));
  GP_IncCounter(GP_GetCounter("d"));
  GP_Counter b = GP_GetCounter("b");
  GP_AddCounter(GP_GetCounter("c"), 21);
  GP_IncCounter(GP_GetCounter("b"));
  GP_IncCounter(b);

  fail_unless(GP_CounterVal(GP_GetCounter("a")) == 2);
  fail_unless(GP_CounterVal(GP_GetCounter("b")) == -6);
  fail_unless(GP_CounterVal(GP_GetCounter("c")) == 21);
  fail_unless(GP_CounterVal(GP_GetCounter("d")) == 1);
  fail_unless(GP_CounterVal(GP_GetCounter("e")) == -42);
}

GP_TEST(Overflow)
{
  char buf[8];
  for (int i = 0; i < GP_COUNTER_MAX + 10; i++) {
    sprintf(buf, "%d", i);
    GP_GetCounter(buf);
  }
  fail_unless(GP_GetCounter("") == NULL);
}

