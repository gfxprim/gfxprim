// SPDX-License-Identifier: GPL-2.1-or-later
/*
 * Copyright (C) 2009-2013 Cyril Hrubis <metan@ucw.cz>
 */

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
