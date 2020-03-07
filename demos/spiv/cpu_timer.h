// SPDX-License-Identifier: GPL-2.1-or-later
/*
 * Copyright (C) 2009-2012 Cyril Hrubis <metan@ucw.cz>
 */

 /*

   Simple timers to count and print cpu time spend in particular codepath.

  */

#ifndef __CPU_TIMER_H__
#define __CPU_TIMER_H__

#include <time.h>

struct cpu_timer {
	struct timespec t_cpu_start;
	struct timespec t_cpu_stop;
	struct timespec t_real_start;
	struct timespec t_real_stop;
	const char *name;
};

/*
 * Enable/disable all timers.
 */
void cpu_timer_switch(int enable);

/*
 * Inialize cpu timer.
 */
void cpu_timer_start(struct cpu_timer *self, const char *name);

/*
 * Stops cpu timer and prints result.
 */
void cpu_timer_stop(struct cpu_timer *self);

#endif /* __CPU_TIMER_H__ */
