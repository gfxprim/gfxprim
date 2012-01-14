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
 * Copyright (C) 2012      Cyril Hrubis <metan@ucw.cz>                       *
 *                                                                           *
 *****************************************************************************/

#include <string.h>
#include "GP_Common.h"
#include "GP_Counter.h"

/*
 * Internal struct used in counter list
 */
struct GP_CounterRecord {
	char name[GP_COUNTER_NAME_LEN];
	GP_Counter counter;
};

/* 
 * variables local to module (static) 
 */
#ifdef	GP_IMPLEMENT_COUNTERS

static GP_Counter_t GP_counters[GP_COUNTER_MAX];
static int GP_used_counters = 0;
static struct GP_CounterRecord GP_counter_list[GP_COUNTER_MAX];
static GP_Counter_t GP_counter_list_overflow = 0;

#endif	/* GP_IMPLEMENT_COUNTERS */

void GP_PrintCounters(FILE *f)
{
#ifdef	GP_IMPLEMENT_COUNTERS
	int i;
	GP_CHECK(f != NULL);
	if (GP_used_counters == 0)
		fprintf(f, "[ no counters defined ]\n");
	for (i = 0; i < GP_used_counters; i++) 
		fprintf(f, "%*s : %llu\n", -GP_COUNTER_NAME_LEN, 
			GP_counter_list[i].name,
			(long long unsigned int)*(GP_counter_list[i].counter));
	if (GP_counter_list_overflow > 0)
		fprintf(f, "[ unable to allocate new counter %llu times ]\n",
			(long long unsigned int)GP_counter_list_overflow);
#endif	/* GP_IMPLEMENT_COUNTERS */
}

GP_Counter GP_GetCounter(const char *name)
{
#ifdef	GP_IMPLEMENT_COUNTERS
	int l = 0;
	int r = GP_used_counters;

	GP_CHECK(name != NULL);
	GP_CHECK(strlen(name) + 1 <= GP_COUNTER_NAME_LEN);

	/* 
	 * Bisect GP_counter_list to find either the counter or a place for it 
	 * interval [l, r) (not incl. r)
	 */
	while (r > l) {
		int med = (r + l) / 2; /* Here never equal to r, might be l */
		int cmp = strcmp(GP_counter_list[med].name, name);
		if (cmp == 0)
			return GP_counter_list[med].counter;
		if (cmp < 0)
			l = med + 1;
		else
			r = med;
	}
	GP_CHECK(l == r);
	if ((l < GP_used_counters) && (strcmp(GP_counter_list[l].name, name) == 0))
		return GP_counter_list[l].counter;

	/* Add a new counter */
	if (GP_used_counters >= GP_COUNTER_MAX) {
		GP_counter_list_overflow++;
		return NULL;
	}

	/* Move the counter records up and initialize a new one */
	memmove(GP_counter_list + l + 1, GP_counter_list + l, 
		sizeof(struct GP_CounterRecord) * GP_used_counters - l);
	strcpy(GP_counter_list[l].name, name);
	GP_counter_list[l].counter = GP_counters + GP_used_counters;
	
	GP_used_counters++;
	return GP_counter_list[l].counter;
#else	/* GP_IMPLEMENT_COUNTERS */
	return NULL;
#endif	/* GP_IMPLEMENT_COUNTERS */
}
