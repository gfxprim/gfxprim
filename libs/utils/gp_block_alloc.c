//SPDX-License-Identifier: LGPL-2.0-or-later

/*

   Copyright (c) 2014-2020 Cyril Hrubis <metan@ucw.cz>

 */

#include "../../config.h"

#ifdef HAVE_MMAP
# include <sys/mman.h>
#endif
#include <core/gp_debug.h>
#include <utils/gp_block_alloc.h>

static size_t block_size;

static size_t align(size_t size)
{
	size_t mask = 3;

	return (size + mask) & ~mask;
}

static void *alloc_block(void)
{
#ifdef HAVE_MMAP
	return mmap(NULL, block_size, PROT_READ|PROT_WRITE,
	            MAP_ANONYMOUS|MAP_PRIVATE, -1, 0);
#else
	return malloc(block_size);
#endif
}

static void free_block(void *addr)
{
#ifdef HAVE_MMAP
	munmap(addr, block_size);
#else
	free(addr);
#endif
}

static gp_balloc_pool *new_block(void)
{
	gp_balloc_pool *block;

	if (!block_size)
		block_size = getpagesize();

	GP_DEBUG(1, "Allocating block size %zu", block_size);

	block = alloc_block();
	if (!block)
		return NULL;

	block->next = NULL;
	block->free = block_size - sizeof(gp_balloc_pool);

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
	(*self)->free = block_size - sizeof(gp_balloc_pool);
}

static void *do_alloc(gp_balloc_pool *self, size_t size)
{
	size_t pos = block_size - self->free;

	self->free -= size;

	return (char*)self + pos;
}

void *gp_balloc(gp_balloc_pool **self, size_t size)
{
	gp_balloc_pool *block;

	if (!*self) {
		GP_DEBUG(1, "Initializing empty block allocator");
		*self = new_block();
		if (!*self)
			return NULL;
	}

	size = align(size);

	for (block = *self; block; block = block->next) {
		if (block->free >= size) {
			GP_DEBUG(2, "Allocating %zu from block %p free %zu",
			         size, block, block->free);
			return do_alloc(block, size);
		}
	}

	GP_DEBUG(2, "Allocating new block size %zu", size);

	block = new_block();
	if (!block)
		return NULL;

	block->next = *self;
	*self = block;

	return do_alloc(block, size);
}
