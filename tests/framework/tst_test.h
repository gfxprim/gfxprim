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
 * Copyright (C) 2009-2012 Cyril Hrubis <metan@ucw.cz>                       *
 *                                                                           *
 *****************************************************************************/

#ifndef TST_TEST_H
#define TST_TEST_H

enum tst_ret {
	TST_SUCCESS, /* Test succedded                                */
	TST_INTERR,  /* Test framework error                          */
	TST_SIGSEGV, /* Test ended with SIGSEGV                       */
	TST_TIMEOUT, /* Test hasn't finished in time                  */
	TST_ABORTED, /* The abort() was called (possible double free) */
	TST_MEMLEAK, /* Memory leak was detected                      */
	TST_FAILED,  /* Test failed                                   */
	TST_MAX = TST_FAILED+1,
};

enum tst_flags {
	/* 
	 * Create teporary directory and set it as CWD and
	 * clean it after the test is finised.
	 */
	TST_TMPDIR = 0x01,

	/*
	 * Check malloc for memory leaks.
	 */
	TST_CHECK_MALLOC = 0x02,
};

struct tst_test {
	const char *name;
	/* test function */
	int (*tst_fn)(void);
	/* time limit in seconds 0 == unlimited */
	unsigned int timeout;
	/* test flags */
	int flags;
};

struct tst_suite {
	const char *suite_name;
	struct tst_test tests[];
};

/*
 * Runs test suite.
 */
void tst_run_suite(const struct tst_suite *suite, const char *tst_name);

/*
 * Lists all suite tests.
 */
void tst_list_suite(const struct tst_suite *suite);

/*
 * Printf-like reporting function.
 */
int tst_report(int level, const char *fmt, ...)
	__attribute__ ((format (printf, 2, 3)));

int tst_warn(const char *fmt, ...)
	__attribute__ ((format (printf, 1, 2)));

#endif /* TST_TEST_H */
