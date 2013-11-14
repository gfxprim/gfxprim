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
 * Copyright (C) 2009-2013 Cyril Hrubis <metan@ucw.cz>                       *
 *                                                                           *
 *****************************************************************************/

#ifndef TST_PRELOAD_H
#define TST_PRELOAD_H

/*
 * Starts malloc check.
 */
void tst_malloc_check_start(void);

/*
 * Stops malloc check.
 */
void tst_malloc_check_stop(void);

enum tst_malloc_canary {
	MALLOC_CANARY_OFF,
	/* canary before the chunk */
	MALLOC_CANARY_BEGIN,
	/* canary after the chunk */
	MALLOC_CANARY_END,
};

/*
 * Turns on malloc canaries.
 *
 * If turned on each malloc gets a canary (a page with both read and write
 * turned off) allocated at the end or at the start of the allocated chunk.
 *
 * Canaries cannot be turned on when malloc checking is off, and malloc checking
 * cannot be turned off when canaries are turned on.
 */
void tst_malloc_canaries_set(enum tst_malloc_canary canary);

struct malloc_stats {
	/*
	 * Sum of all alocated chunks.
	 */
	size_t total_size;
	unsigned int total_chunks;

	/*
	 * Maximal allocated size and number of chunks at any time.
	 */
	size_t max_size;
	unsigned int max_chunks;

	/*
	 * Memory that was allocated but not freed
	 */
	size_t lost_size;
	unsigned int lost_chunks;
};

/*
 * Reports current malloc status.
 *
 * Size and chunks are filled with sum and number of currently allocated
 * chunks, i.e, chunks that were allocated but not freed. The allocs is
 * filled with number of allocations done.
 */
void tst_malloc_check_report(struct malloc_stats *stats);

/*
 * Prints malloc statistics.
 */
void tst_malloc_print(const struct malloc_stats *stats);

#endif /* TST_PRELOAD_H */
