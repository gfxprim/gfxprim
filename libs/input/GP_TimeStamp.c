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
 * Copyright (C) 2009-2013 Cyril Hrubis <metan@ucw.cz>                       *
 *                                                                           *
 *****************************************************************************/

#include <errno.h>
#include <string.h>
#include <time.h>

#include "core/GP_Common.h"
#include "core/GP_Debug.h"

#include "input/GP_TimeStamp.h"

#define MS_IN_US 1000000

/*
 * Chooses fastest clock with milisecond resolution.
 */
static int choose_clock(clockid_t *clock)
{
	struct timespec ts;

#ifdef CLOCK_MONOTONIC_COARSE
	GP_DEBUG(1, "Trying CLOCK_MONOTONIC_COARSE");

	if (clock_getres(CLOCK_MONOTONIC_COARSE, &ts)) {
		GP_DEBUG(1, "CLOCK_MONOTONIC_COARSE: %s", strerror(errno));
	} else {
		GP_DEBUG(1, "CLOCK_MONOTONIC_COARSE resolution is %lis %lins",
		         (long)ts.tv_sec, ts.tv_nsec);

		/* Depends on CONFIG_HZ */
		if (ts.tv_sec == 0 && ts.tv_nsec <= MS_IN_US) {
			*clock = CLOCK_MONOTONIC_COARSE;
			return 0;
		}
	}
#endif

	if (clock_getres(CLOCK_MONOTONIC, &ts)) {
		GP_DEBUG(1, "CLOCK_MONOTONIC: %s", strerror(errno));
		return 1;
	}

	GP_DEBUG(1, "CLOCK_MONOTONIC resulution is %lis %lins",
	         (long)ts.tv_sec, ts.tv_nsec);

	if (ts.tv_sec == 0 && ts.tv_nsec <= MS_IN_US) {
		*clock = CLOCK_MONOTONIC;
		return 0;
	}

	GP_DEBUG(1, "No suitable clock id found.");
	//TODO: Fallback to gettimeofday?

	return 1;
}

uint64_t GP_GetTimeStamp(void)
{
	struct timespec ts;
	static clockid_t clock = -1;

	if (clock == -1)
		choose_clock(&clock);

	if (clock_gettime(clock, &ts)) {
		GP_ABORT("clock_gettime(%i) failed unexpectedly: %s",
		         clock, strerror(errno));
	}

	return (uint64_t)ts.tv_sec * 1000 + ts.tv_nsec / MS_IN_US;
}
