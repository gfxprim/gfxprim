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

#include <stdio.h>
#include "cpu_timer.h"

static void to_time(int *sec, int *nsec, struct timespec *start,
                    struct timespec *stop)
{
	if (stop->tv_nsec < start->tv_nsec) {
		*sec  = stop->tv_sec - start->tv_sec - 1;
		*nsec = stop->tv_nsec + 1000000000 - start->tv_nsec;
	} else {
		*sec  = stop->tv_sec  - start->tv_sec;
		*nsec = stop->tv_nsec - start->tv_nsec;
	}
}

void cpu_timer_start(struct cpu_timer *self, const char *name)
{
	self->name = name;
	
	clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &self->t_cpu_start);
	clock_gettime(CLOCK_MONOTONIC, &self->t_real_start);
}

void cpu_timer_stop(struct cpu_timer *self)
{
	clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &self->t_cpu_stop);
	clock_gettime(CLOCK_MONOTONIC, &self->t_real_stop);

	int cpu_sec;
	int cpu_nsec;

	int real_sec;
	int real_nsec;

	to_time(&cpu_sec, &cpu_nsec, &self->t_cpu_start, &self->t_cpu_stop);
	to_time(&real_sec, &real_nsec, &self->t_real_start, &self->t_real_stop);

	printf("TIMER '%s' CPU=%i.%09is REAL=%i.%09is\n", self->name,
	       cpu_sec, cpu_nsec, real_sec, real_nsec);
}
