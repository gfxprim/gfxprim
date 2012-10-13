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
  
   Timespec manipulation utils.

  */

#ifndef TST_TIMESPEC_H
#define TST_TIMESPEC_H

#include <time.h>

double timespec_to_double(const struct timespec *t);

void double_to_timespec(const double time, struct timespec *res);

void timespec_sub(const struct timespec *a, const struct timespec *b,
                  struct timespec *res);

void timespec_add(const struct timespec *a, struct timespec *res);

void timespec_div(struct timespec *res, unsigned int div);

#endif /* TST_TIMESPEC_H */
