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

#define _GNU_SOURCE
#include <stdio.h>
#include <sys/mman.h>
#include <stdlib.h>
#include <unistd.h>
#include <dlfcn.h>

#include "tst_malloc_canaries.h"

void *tst_malloc_canary_right(size_t size)
{
	size_t pagesize = sysconf(_SC_PAGESIZE);
	size_t pages = size/pagesize + !!(size%pagesize) + 1;
	char *buf;

	if (size == 0)
		return NULL;

	if (posix_memalign((void*)&buf, pagesize, pages * pagesize))
		return NULL;

	/*
	 * Turn off read/write acces on the last page. The buffer starts
	 * somewhere in the first page so that the end is exactly before the
	 * start of the last page.
	 */
	if (mprotect(buf + pagesize * (pages - 1), pagesize, PROT_NONE)) {
		perror("mprotect");
		free(buf);
		return NULL;
	}

	if (size%pagesize)
		return buf + (pagesize - size%pagesize);

	return buf;
}

static void (*real_free)(void *) = NULL;

void tst_free_canary_right(void *ptr, size_t size)
{
	size_t pagesize = sysconf(_SC_PAGESIZE);
	size_t pages = size/pagesize + !!(size%pagesize);
	void *start = size%pagesize ? (char*)ptr - (pagesize - size%pagesize) : ptr;

	/* Reset the memory protection back to RW */
	if (mprotect(start + pagesize * pages, pagesize, PROT_READ | PROT_WRITE))
		perror("mprotect");

	if (!real_free)
		real_free = dlsym(RTLD_NEXT, "free");

	real_free(start);
}

void *tst_malloc_canary_left(size_t size)
{
	size_t pagesize = sysconf(_SC_PAGESIZE);
	size_t pages = size/pagesize + !!(size%pagesize) + 1;

	if (size == 0)
		return NULL;

	char *buf;

	if (posix_memalign((void*)&buf, pagesize, pages * pagesize))
		return NULL;

	/*
	 * Turn off read/write acces on the first page, the buffer starts right
	 * after it.
	 */
	if (mprotect(buf, pagesize, PROT_NONE)) {
		perror("mprotect");
		free(buf);
		return NULL;
	}

	return buf + pagesize;
}

void tst_free_canary_left(void *ptr, size_t size __attribute__((unused)))
{
	size_t pagesize = sysconf(_SC_PAGESIZE);
	void *start = ptr - pagesize;

	/* Reset the memory protection back to RW */
	if (mprotect(start, pagesize, PROT_READ | PROT_WRITE)) {
		printf("%p size %zu\n", ptr, size);
		perror("mprotect");
	}

	if (!real_free)
		real_free = dlsym(RTLD_NEXT, "free");

	real_free(start);
}
