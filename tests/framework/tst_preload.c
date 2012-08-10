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

#define _GNU_SOURCE
#include <stdio.h>
#include <stdint.h>
#include <dlfcn.h>

#include "tst_test.h"
#include "tst_preload.h"

static int check_malloc = 0;

void tst_malloc_check_start(void)
{
	check_malloc = 1;
}

void tst_malloc_check_stop(void)
{
	check_malloc = 0;
}

#define MAX_CHUNKS 100

struct chunk {
	void *ptr;
	size_t size;
	int cont;
};

struct chunk chunks[MAX_CHUNKS + 1];
static size_t total_size;
static unsigned int total_chunks;

void tst_malloc_check_report(size_t *size, unsigned int *chunks)
{
	*size = total_size;
	*chunks = total_chunks;
}

static void add_chunk(size_t size, void *ptr)
{
	int i;

	for (i = 0; i < MAX_CHUNKS; i++) {
		if (chunks[i].size == 0) {
			chunks[i].size = size;
			chunks[i].ptr = ptr;
			total_size += size;
			total_chunks += 1;
			return;
		}
	}

	tst_warn("Not enough chunks (%i) for malloc() tracing", MAX_CHUNKS);
}

static void rem_chunk(void *ptr)
{
	int i;

	for (i = 0; i < MAX_CHUNKS; i++) {
		/* Nothing interesting in the rest of the array */
		if (chunks[i].size == 0 && chunks[i].cont == 0)
			break;

		if (chunks[i].ptr == ptr) {
			total_size -= chunks[i].size;
			total_chunks -= 1;
			
			chunks[i].size = 0;
			chunks[i].ptr = NULL;
			
			if (chunks[i+1].size != 0 || chunks[i+1].cont != 0)
				chunks[i].cont = 1;
			return;
		}
	}

	tst_warn("Chunk passed to free not found (%p)", ptr);
}

void *malloc(size_t size)
{
	static void *(*real_malloc)(size_t) = NULL;

	if (!real_malloc)
		real_malloc = dlsym(RTLD_NEXT, "malloc");

	void *ptr = real_malloc(size);

	if (check_malloc && ptr != NULL)
		add_chunk(size, ptr);

	return ptr;
}

void free(void *ptr)
{
	static void (*real_free)(void *) = NULL;

	if (!real_free)
		real_free = dlsym(RTLD_NEXT, "free");
	
	if (check_malloc && ptr != NULL)
		rem_chunk(ptr);

	real_free(ptr);
}
