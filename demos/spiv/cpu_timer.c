// SPDX-License-Identifier: GPL-2.1-or-later
/*
 * Copyright (C) 2009-2012 Cyril Hrubis <metan@ucw.cz>
 */

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

static int timers_disabled = 1;

void cpu_timer_switch(int enable)
{
	timers_disabled = !enable;
}

void cpu_timer_start(struct cpu_timer *self, const char *name)
{
	if (timers_disabled)
		return;

	self->name = name;

	clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &self->t_cpu_start);
	clock_gettime(CLOCK_MONOTONIC, &self->t_real_start);
}

void cpu_timer_stop(struct cpu_timer *self)
{
	if (timers_disabled)
		return;

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
