//SPDX-License-Identifier: LGPL-2.0-or-later

/*

   Copyright (c) 2014-2026 Cyril Hrubis <metan@ucw.cz>

 */

#include "../../config.h"

#ifdef HAVE_MMAP
# include <sys/mman.h>
#endif
#include <core/gp_common.h>
#include <core/gp_debug.h>
#include <utils/gp_block_alloc.h>

static size_t align(size_t size)
{
	size_t mask = 3;

	return (size + mask) & ~mask;
}

static void *alloc_block(size_t size)
{
#ifdef HAVE_MMAP
	void *ret = mmap(NULL, size, PROT_READ|PROT_WRITE,
	                 MAP_ANONYMOUS|MAP_PRIVATE, -1, 0);

	return ret == MAP_FAILED ? NULL : ret;
#else
	return malloc(size);
#endif
}

static void free_block(gp_balloc_pool *block)
{
#ifdef HAVE_MMAP
	munmap(block, block->size);
#else
	free(block);
#endif
}

#define BLOCK_SIZE_CAP_PAGES 256

static gp_balloc_pool *new_block(size_t min_size, size_t prev_size)
{
	size_t page_size = getpagesize();
	size_t size = prev_size ? 2 * prev_size : page_size;
	gp_balloc_pool *block;

	size = GP_MIN(size, BLOCK_SIZE_CAP_PAGES * page_size);

	while (size - sizeof(gp_balloc_pool) < min_size)
		size *= 2;

	GP_DEBUG(1, "Allocating block size %zu", size);

	block = alloc_block(size);
	if (!block)
		return NULL;

	block->next = NULL;
	block->free = size - sizeof(gp_balloc_pool);
	block->size = size;

	return block;
}

void gp_bfree(gp_balloc_pool **self)
{
	gp_balloc_pool *i, *j;

	for (i = *self; i;) {
		j = i->next;
		free_block(i);
		i = j;
	}

	*self = NULL;
}

void gp_bclear(gp_balloc_pool **self)
{
	gp_balloc_pool *i, *j;

	GP_DEBUG(1, "Clearing block allocator %p", *self);

	if (!(*self))
		return;

	for (i = (*self)->next; i;) {
		j = i->next;
		free_block(i);
		i = j;
	}

	(*self)->next = NULL;
	(*self)->free = (*self)->size - sizeof(gp_balloc_pool);
}

static void *do_alloc(gp_balloc_pool *self, size_t size)
{
	size_t pos = self->size - self->free;

	self->free -= size;

	return (char*)self + pos;
}

void *gp_balloc(gp_balloc_pool **self, size_t size)
{
	gp_balloc_pool *block;

	size = align(size);

	if (!*self) {
		GP_DEBUG(1, "Initializing empty block allocator");
		*self = new_block(size, 0);
		if (!*self)
			return NULL;
	}

	for (block = *self; block; block = block->next) {
		if (block->free >= size) {
			GP_DEBUG(2, "Allocating %zu from block %p free %zu",
			         size, block, block->free);
			return do_alloc(block, size);
		}
	}

	GP_DEBUG(2, "Allocating new block size %zu", size);

	block = new_block(size, (*self)->size);
	if (!block)
		return NULL;

	block->next = *self;
	*self = block;

	return do_alloc(block, size);
}
