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

 /*

   Simple timers to count cpu time.

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
 * Inialize cpu timer.
 */
void cpu_timer_start(struct cpu_timer *self, const char *name);

/*
 * Stops cpu timer and prints result.
 */
void cpu_timer_stop(struct cpu_timer *self);

#endif /* __CPU_TIMER_H__ */
