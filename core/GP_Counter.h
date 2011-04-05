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
 * Copyright (C) 2011      Tomas Gavenciak <gavento@ucw.cz>                  *
 *                                                                           *
 *****************************************************************************/

#ifndef GP_COUNTER_H
#define GP_COUNTER_H

/* 
 * Simple global named 64-bit counters.
 *
 * Intended for accounting of expensive operations ("quantitative warnings").
 * All operations accept NULL as GP_Counter.
 *
 * If GP_IMPLEMENT_COUNTERS is undefined, all operations are NOP and
 * no memory is allocated.
 */

#define GP_IMPLEMENT_COUNTERS

/*
 * Maximum length of counter name,
 * maximum number of counters (~40 bytes each) 
 */

#define GP_COUNTER_NAME_LEN 24
#define GP_COUNTER_MAX 256

/* 
 * Basic types. 
 * Only use GP_Counter in your programs. 
 */

typedef int64_t GP_Counter_t;
typedef GP_Counter_t *GP_Counter;

/* 
 * Increase a counter by 1.
 */

static inline void GP_IncCounter(GP_Counter counter) 
{
#ifdef	GP_IMPLEMENT_COUNTERS
	if (!counter) return;
	(*counter) ++;
#endif	/* GP_IMPLEMENT_COUNTERS */
}

/*
 * Increase a counter by delta (may be negative).
 * No checks for underflow.
 */

static inline void GP_AddCounter(GP_Counter counter, GP_Counter_t delta) 
{
#ifdef	GP_IMPLEMENT_COUNTERS
	if (!counter) return;
	(*counter) += delta;
#endif	/* GP_IMPLEMENT_COUNTERS */
}

/*
 * Set counter to given value.
 */

static inline void GP_SetCounter(GP_Counter counter, GP_Counter_t value) 
{
#ifdef	GP_IMPLEMENT_COUNTERS
	if (!counter) return;
	(*counter) = value;
#endif	/* GP_IMPLEMENT_COUNTERS */
}

/*
 * Return counter value
 */

inline GP_Counter_t GP_CounterVal(GP_Counter counter)
{
#ifdef	GP_IMPLEMENT_COUNTERS
	if (!counter) return 0;
	return *counter;
#else	/* GP_IMPLEMENT_COUNTERS */
	return 0;
#endif  /* GP_IMPLEMENT_COUNTERS */
}

/*
 * Pretty-printing of all counters and their values to f
 * Includes info about counter-list overflow
 */

struct FILE;
void GP_PrintCounters(struct FILE *f);

/*
 * Lookup a counter by name, possibly creating a new one.
 *
 * May return NULL if no space is left in the counter list.
 * The returned value is not unallocated in any way.
 *
 * NOTE: Current implementation has very slow adds (O(current_counters)),
 *	 but the lookup is reasonably fast (bisection)
 */

GP_Counter GP_GetCounter(const char *name);

#endif	/* GP_COUNTER_H */
