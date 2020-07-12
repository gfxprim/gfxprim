//SPDX-License-Identifier: LGPL-2.0-or-later

/*

   Copyright (c) 2014-2020 Cyril Hrubis <metan@ucw.cz>

   This is simple allocator for cases where we want to free all memory at once.
   In contrast to malloc() this does not maintain any comples data structures
   and freeing memory is much faster.

 */

#ifndef GP_BLOCK_ALLOC_H__
#define GP_BLOCK_ALLOC_H__

typedef struct gp_block {
	struct gp_block *next;
	size_t free;
} gp_block;

/*
 * @brief Allocate memory from a block pool.
 *
 * The pointer to the gp_block has to be set to NULL prior first call to this
 * function.
 *
 * @self A pointer to a block pointer.
 * @size A memory size to be allocated.
 *
 * @return A pointer to allocated memory, returns NULL on a failure.
 */
void *gp_block_alloc(gp_block **self, size_t size);

/*
 * @brief Free all blocks in the allocator.
 *
 * @self A pointer to a block pointer.
 */
void gp_block_free(gp_block **self);

#endif /* GP_BLOCK_ALLOC_H__ */
