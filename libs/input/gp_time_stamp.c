// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2009-2013 Cyril Hrubis <metan@ucw.cz>
 */

#include <errno.h>
#include <string.h>
#include <time.h>

#include "core/gp_common.h"
#include <core/gp_debug.h>

#include <input/gp_time_stamp.h>

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
		//TODO: Fallback to gettimeofday?
		return 1;
	}

	GP_DEBUG(1, "CLOCK_MONOTONIC resulution is %lis %lins",
	         (long)ts.tv_sec, ts.tv_nsec);

	if (ts.tv_sec == 0 && ts.tv_nsec <= MS_IN_US) {
		*clock = CLOCK_MONOTONIC;
		return 0;
	}

	if (ts.tv_sec) {
		GP_FATAL("No suitable clock found");
		return 1;
	}

	GP_WARN("Timers running with %lims granularity",
                ts.tv_nsec/MS_IN_US);

	*clock = CLOCK_MONOTONIC;

	return 1;
}

uint64_t gp_time_stamp(void)
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
