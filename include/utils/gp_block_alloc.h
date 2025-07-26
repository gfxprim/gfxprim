//SPDX-License-Identifier: LGPL-2.0-or-later

/*

   Copyright (c) 2014-2025 Cyril Hrubis <metan@ucw.cz>

 */

 /**
  * @file gp_block_alloc.h
  * @brief Simple block allocator.
  *
  * This is simple allocator for cases where we want to free all memory at once.
  *
  * The allocator allocates memory in size pre-defined blocks and hands out
  * memory from such pools until the block memory is exhausted. The allocator
  * does not have to maintain free blocks, since all memory is freed at once,
  * which makes the code super simple and fast.
  */

#ifndef GP_BLOCK_ALLOC_H
#define GP_BLOCK_ALLOC_H

#include <stddef.h>
#include <core/gp_compiler.h>

/** @brief A block allocator block header. */
typedef struct gp_balloc_pool {
	/** @brief A pointer to a next block. */
	struct gp_balloc_pool *next;
	/** @brief Free space in bytes in the block. */
	size_t free;
} gp_balloc_pool;

/**
 * @brief Allocate memory from a block pool.
 *
 * The pointer to the gp_balloc has to be set to NULL prior first call to this
 * function.
 *
 * @param self A pointer to a block pointer.
 * @param size A memory size to be allocated.
 *
 * @return A pointer to allocated memory, returns NULL on a failure.
 */
GP_WUR void *gp_balloc(gp_balloc_pool **self, size_t size);

/**
 * @brief Free all blocks in the allocator.
 *
 * @param self A pointer to a block pointer.
 */
void gp_bfree(gp_balloc_pool **self);

/**
 * @brief Clears all the data in the block allocator.
 *
 * This is similar to gp_bfree() but keeps exacly one allocated and cleared
 * block for reuse.
 *
 * @param self A pointer to a block pointer.
 */
void gp_bclear(gp_balloc_pool **self);

#endif /* GP_BLOCK_ALLOC_H */
