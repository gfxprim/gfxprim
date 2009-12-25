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

#ifndef __TEST_UTILS_H__
#define __TEST_UTILS_H__

/*
 * Create these in your test.
 */
extern const char *TST_TestName;
extern const char *TST_TestDesc;
extern const int   TST_TestCases;

#define TST_OK   0x00 /* test was ok           */
#define TST_FAIL 0x01 /* test has failed       */
#define TST_EXIT 0x04 /* exit the test         */
#define TST_STAT 0x08 /* print time statistics */

/*
 * Initalize the test.
 */
void TST_Init(void);

/*
 * Starts test counters for operation.
 */
void TST_Start(const char *CaseName);

/*
 * Stop test counters and print result.
 */
void TST_Stop(int TST_flag, const char *format, ...) __attribute__ ((format (printf, 2, 3)));

/*
 * Exit test suite.
 */
void TST_Exit(void) __attribute__ ((noreturn));


/*
 * Do not use, for internal purposes only
 */
struct timeval;

struct TST_TestData {
	struct timeval TestStartTime;
	struct timeval Duration;
	struct timeval StartTime;
	int TestCases;

	int fails;
};

extern struct TST_TestData TST_TestData;

#endif /* __TEST_UTILS_H__ */
