// SPDX-License-Identifier: GPL-2.1-or-later
/*
 * Copyright (C) 2009-2013 Cyril Hrubis <metan@ucw.cz>
 */

#define _GNU_SOURCE
#include <stdio.h>
#include <sys/mman.h>
#include <stdlib.h>
#include <unistd.h>
#include <dlfcn.h>

#include "tst_malloc_canaries.h"

void *tst_malloc_canary_right(size_t size)
{
	size_t pagesize = getpagesize();
	size_t pages = size/pagesize + !!(size%pagesize) + 1;
	void *ret;

	if (size == 0)
		return NULL;

	ret = mmap(NULL, pages * pagesize, PROT_READ | PROT_WRITE,
	           MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);

	if (ret == MAP_FAILED)
		return NULL;

	/*
	 * Turn off read/write acces on the last page. The buffer starts
	 * somewhere in the first page so that the end is exactly before the
	 * start of the last page.
	 */
	if (mprotect(ret + (pages - 1) * pagesize, pagesize, PROT_NONE)) {
		perror("mprotect");
		munmap(ret, pages * pagesize);
		return NULL;
	}

	if (size % pagesize)
		return ret + (pagesize - size%pagesize);

	return ret;
}

void tst_free_canary_right(void *ptr, size_t size)
{
	size_t pagesize = getpagesize();
	size_t pages = size/pagesize + !!(size%pagesize);
	void *start = size%pagesize ? ptr - (pagesize - size%pagesize) : ptr;

	munmap(start, pages * pagesize);
}

void *tst_malloc_canary_left(size_t size)
{
	size_t pagesize = sysconf(_SC_PAGESIZE);
	size_t pages = size/pagesize + !!(size%pagesize) + 1;
	void *ret;

	if (size == 0)
		return NULL;

	ret = mmap(NULL, pages * pagesize, PROT_READ | PROT_WRITE,
	           MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);

	if (ret == MAP_FAILED)
		return NULL;

	/*
	 * Turn off read/write acces on the first page, the buffer starts right
	 * after it.
	 */
	if (mprotect(ret, pagesize, PROT_NONE)) {
		perror("mprotect");
		munmap(ret, pages * pagesize);
		return NULL;
	}

	return ret + pagesize;
}

void tst_free_canary_left(void *ptr, size_t size __attribute__((unused)))
{
	size_t pagesize = sysconf(_SC_PAGESIZE);
	size_t pages = size/pagesize + !!(size%pagesize) + 1;
	void *start = ptr - pagesize;

	munmap(start, pages * pagesize);
}
