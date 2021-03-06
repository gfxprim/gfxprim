// SPDX-License-Identifier: GPL-2.1-or-later
/*
 * Copyright (C) 2009-2012 Cyril Hrubis <metan@ucw.cz>
 */

 /*

   Timespec manipulation utils.

  */

#ifndef TST_TIMESPEC_H
#define TST_TIMESPEC_H

#include <time.h>

void timespec_diff(int *sec, int *nsec,
                   const struct timespec *start,
                   const struct timespec *stop);

double timespec_to_double(const struct timespec *t);

void double_to_timespec(const double time, struct timespec *res);

void timespec_sub(const struct timespec *a, const struct timespec *b,
                  struct timespec *res);

void timespec_add(const struct timespec *a, struct timespec *res);

void timespec_div(struct timespec *res, unsigned int div);

#endif /* TST_TIMESPEC_H */
