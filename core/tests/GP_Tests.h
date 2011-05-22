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

#include <check.h>

/*
 * Helper macro to allow auto-generation of test-cases and suites.
 * Searched for by find_tests.py.
 *
 * Use alone on a line as
 *   GP_TEST(testname) 
 * or
 *   GP_TEST(testname, "foo=1, bar='baz'")
 * The optional string is passed as parameters to Python dict() as parameters
 * for the testcase-generator.
 * Currently, the following parameters are recognized:
 *  suite         -- name of the suite
 *  loop_start    -- test will be repeated as with:
 *  loop_end         for (int _i = loop_start; _i < loop_end; i++) { ... }
 *  expect_exit   -- expect the function to exit with given exitcode
 *  expect_signal -- expect the function to exit with given exitcode
 *
 * Parameters name, fname, line are used internally, do not use them
 */

#define GP_TEST(name, ...) static void name(int);\
			   void GP_TEST_##name(int i) {name(i);} \
			    START_TEST(name)

/*
 * Helper macro to allow auto-generation of suites. 
 * Defines suite from this point until EOF or redefinition.
 * Searched for by find_tests.py
 *
 * Use alone on a line as
 *   GP_SUITE(suitename) 
 */

#define GP_SUITE(name, ...)

