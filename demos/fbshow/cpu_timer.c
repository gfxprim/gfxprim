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

void cpu_timer_start(struct cpu_timer *self, const char *name)
{
	self->name = name;
	clock_gettime(CLOCK_THREAD_CPUTIME_ID, &self->t_start);
}

void cpu_timer_stop(struct cpu_timer *self)
{
	clock_gettime(CLOCK_THREAD_CPUTIME_ID, &self->t_stop);

	int sec;
	int nsec;

	if (self->t_stop.tv_nsec < self->t_start.tv_nsec) {
		sec  = self->t_stop.tv_sec - self->t_start.tv_sec - 1;
		nsec = self->t_stop.tv_nsec + 1000000000 - self->t_start.tv_nsec;
	} else {
		sec  = self->t_stop.tv_sec  - self->t_start.tv_sec;
		nsec = self->t_stop.tv_nsec - self->t_start.tv_nsec;
	}

	printf("TIMER '%s' %i.%09i sec\n", self->name, sec, nsec);
}
