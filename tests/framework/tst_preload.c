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
 * Copyright (C) 2009-2015 Cyril Hrubis <metan@ucw.cz>                       *
 *                                                                           *
 *****************************************************************************/

#define _GNU_SOURCE
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <dlfcn.h>
#include <execinfo.h>
#include <time.h>

#include "tst_test.h"
#include "tst_preload.h"
#include "tst_malloc_canaries.h"

static int check_malloc = 0;
static int verbose = 0;
static int malloc_canary = MALLOC_CANARY_OFF;

static size_t cur_size = 0;
static unsigned int cur_chunks = 0;
static size_t total_size = 0;
static unsigned int total_chunks = 0;
/* Maximal allocated size at a time */
static size_t max_size = 0;
static unsigned int max_chunks = 0;

void tst_malloc_check_start(void)
{
	void  *buf[1];
	char *str_verbose;

	/*
	 * Call backtrace() before we start tracking memory, because it calls
	 * dlopen() on first invocation, which allocates memory that is never
	 * freed...
	 */
	backtrace(buf, 1);

	/*
	 * And so does mktime(), it allocates memory which is not freed in tzset()
	 *
	 * Unfortunatelly this still leaves one chunk of size 15 that is reallocated
	 * on each call to mktime() :(
	 */
	struct tm tm;
	mktime(&tm);

	check_malloc = 1;

	str_verbose = getenv("TST_MALLOC_VERBOSE");
	if (str_verbose)
		verbose = atoi(str_verbose);
}

static void print_c_trace(void)
{
	void *buffer[128];
	int size = backtrace(buffer, 128);

	fprintf(stderr, "C stack trace (most recent call first):\n");
	fflush(stderr);
	backtrace_symbols_fd(buffer, size, fileno(stderr));
	fprintf(stderr, "\n");
}

void tst_malloc_canaries_set(enum tst_malloc_canary canary)
{
	if (!check_malloc) {
		tst_warn("Cannot turn canaries on when checking is off");
		return;
	}

	if (canary > MALLOC_CANARY_END) {
		tst_warn("Invalid canary type");
		return;
	}

	malloc_canary = canary;
}


void tst_malloc_check_report(struct malloc_stats *stats)
{
	stats->total_size = total_size;
	stats->total_chunks = total_chunks;

	stats->max_size = max_size;
	stats->max_chunks = max_chunks;

	stats->lost_size = cur_size;
	stats->lost_chunks = cur_chunks;
}

#define MAX_CHUNKS 100

struct chunk {
	void *ptr;
	size_t size;
	enum tst_malloc_canary canary;
	int cont;
};

static struct chunk chunks[MAX_CHUNKS];
static unsigned int chunks_top = 0;

static struct chunk *get_chunk(void *ptr)
{
	unsigned int i;

	for (i = 0; i < chunks_top; i++) {
		if (chunks[i].ptr == ptr)
			return &chunks[i];
	}

	return NULL;
}

static void add_chunk(size_t size, void *ptr)
{
	if (chunks_top >= MAX_CHUNKS) {
		tst_warn("Not enough chunks (%i) for malloc() tracing",
		         MAX_CHUNKS);
		return;
	}

	if (get_chunk(ptr))
		tst_warn("Duplicate chunk addres added %p\n", ptr);

	if (verbose > 1)
		fprintf(stderr, " (adding chunk %p %6zu %i)\n", ptr, size, chunks_top);

	/* Store chunk */
	chunks[chunks_top].size = size;
	chunks[chunks_top].ptr = ptr;
	chunks[chunks_top].canary = malloc_canary;
	chunks_top++;

	/* Update global stats */
	cur_size += size;
	cur_chunks++;
	total_size += size;
	total_chunks++;

	if (cur_size > max_size)
		max_size = cur_size;

	if (cur_chunks > max_chunks)
		max_chunks = cur_chunks;
}

static void rem_chunk(void *ptr)
{
	unsigned int i;

	if (verbose > 1)
		fprintf(stderr, " (removing chunk %p)\n", ptr);

	for (i = 0; i < chunks_top; i++) {
		if (chunks[i].ptr == ptr) {
			/* Update global stats */
			cur_size -= chunks[i].size;
			cur_chunks--;

			/* Replace found chunk with top one */
			chunks[i] = chunks[--chunks_top];

			return;
		}
	}

	tst_warn("Chunk passed to free not found (%p)", ptr);
}

void tst_malloc_check_stop(void)
{
	/*
	 * We cannot stop the tracing when canaries are on
	 * because we need the chunk table to keep track
	 * which allocations are with canary and their
	 * sizes for free and realloc.
	 */
	if (malloc_canary != MALLOC_CANARY_OFF) {
		tst_warn("Cannot turn malloc checks off when canaries are on");
		return;
	}

	check_malloc = 0;

	unsigned int i;

	if (verbose) {
		for (i = 0; i < chunks_top; i++) {
			fprintf(stderr, "LOST CHUNK: %p %6zu\n",
			        chunks[i].ptr, chunks[i].size);
		}
	}
}

static void *(*real_malloc)(size_t) = NULL;

void *malloc___(size_t size)
{
	void *ptr;

	if (!real_malloc)
		real_malloc = dlsym(RTLD_NEXT, "malloc");

	switch (malloc_canary) {
	case MALLOC_CANARY_OFF:
		ptr = real_malloc(size);
	break;
	case MALLOC_CANARY_BEGIN:
		ptr = tst_malloc_canary_left(size);
	break;
	case MALLOC_CANARY_END:
		ptr = tst_malloc_canary_right(size);
	break;
	default:
		return NULL;
	}

	if (check_malloc && ptr) {
		if (verbose)
			fprintf(stderr, "MALLOC %p\n", ptr);
		if (verbose > 1)
			print_c_trace();
		add_chunk(size, ptr);
	}

	return ptr;
}

void *calloc___(size_t nmemb, size_t size)
{
	static int been_here = 0;
	void *ptr;

	/*
	 * Fail calloc() before dlsym(RTLD_NEXT, "calloc") returns.
	 *
	 * The glibc seems to work with this failure just fine.
	 */
	if (!real_malloc && been_here)
		return NULL;

	been_here = 1;

	ptr = malloc(nmemb * size);

	if (ptr)
		memset(ptr, 0, nmemb * size);

	return ptr;
}

void free___(void *ptr)
{
	static void (*real_free)(void *) = NULL;
	struct chunk *chunk;

	if (!real_free)
		real_free = dlsym(RTLD_NEXT, "free");

	if (!ptr)
		return;

	chunk = get_chunk(ptr);

	/* Was allocated before malloc checking was turned on */
	if (!chunk) {
		real_free(ptr);
		return;
	}

	switch (chunk->canary) {
	case MALLOC_CANARY_OFF:
		real_free(ptr);
	break;
	case MALLOC_CANARY_BEGIN:
		tst_free_canary_left(ptr, chunk->size);
	break;
	case MALLOC_CANARY_END:
		tst_free_canary_right(ptr, chunk->size);
	break;
	}

	if (verbose)
		fprintf(stderr, "FREE %p\n", ptr);

	rem_chunk(ptr);
}

#define min(a, b) (((a) < (b)) ? (a) : (b))

void *realloc___(void *optr, size_t size)
{
	static void *(*real_realloc)(void*, size_t) = NULL;
	void *ptr;
	struct chunk *chunk;

	if (!real_realloc)
		real_realloc = dlsym(RTLD_NEXT, "realloc");

	if (!optr)
		return malloc(size);

	chunk = get_chunk(optr);

	if (!chunk) {
		/*
		 * We don't know old size -> have to use real_realloc()
		 */
		ptr = real_realloc(optr, size);

		/*
		 * realloc() may call malloc(), add the chunk only if it
		 * haven't been added previously.
		 */
		if (ptr && !get_chunk(ptr))
			add_chunk(size, ptr);

		return ptr;
	}

	ptr = malloc(size);

	if (!ptr)
		return NULL;

	memcpy(ptr, optr, min(chunk->size, size));
	free(optr);

	return ptr;
}

void tst_malloc_print(const struct malloc_stats *stats)
{
	fprintf(stderr, "Total size %zu chunks %u, lost size %zu chunks %u\n",
	        stats->total_size, stats->total_chunks,
		stats->lost_size, stats->lost_chunks);
}
