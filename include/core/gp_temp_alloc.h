// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2009-2024 Cyril Hrubis <metan@ucw.cz>
 */

/**
 * @file gp_temp_alloc.h
 * @brief Temporary block allocator implementation.
 *
 * Creates pool for block allocation (small ones are done on the stack, bigger
 * using malloc).
 *
 * The usage is:
 * @code
 * // Creates new allocation pool
 * gp_temp_alloc_create(buf, 3 * 1024);
 *
 * // Make use of it
 * int *R = gp_temp_alloc_get(buf, 1024);
 * int *G = gp_temp_alloc_get(buf, 1024);
 * int *B = gp_temp_alloc_get(buf, 1024);
 *
 * ...
 *
 * // Free it
 * gp_temp_alloc_free(buf);
 * @endcode
 */

#ifndef CORE_GP_TEMP_ALLOC_H
#define CORE_GP_TEMP_ALLOC_H

#ifdef __linux__
# include <alloca.h>
#endif
#include <stdlib.h>

#include <core/gp_common.h>

#ifndef GP_ALLOCA_THRESHOLD
# define GP_ALLOCA_THRESHOLD 2048
#endif

/**
 * @brief Temporary buffer.
 */
struct gp_temp_alloc {
	/** @brief Buffer start. */
	void *buffer;
	/** @brief How much of buffer was consumed by allocations. */
	size_t pos;
	/** @brief Buffer size. */
	size_t size;
};

#define GP_TEMP_ALLOC(size) ({                                        \
	((size) > GP_ALLOCA_THRESHOLD) ? malloc(size) : alloca(size); \
})

#define gp_temp_alloc_create(name, bsize)                              \
	struct gp_temp_alloc name = {.size = (bsize), .pos = 0,       \
				    .buffer = GP_TEMP_ALLOC(bsize)};

#define gp_temp_alloc_get(self, bsize) ({            \
	GP_ASSERT(self.pos + bsize <= self.size);    \
	size_t gp_pos__ = self.pos;                  \
	self.pos += bsize;                           \
	(void*)(((char*)(self.buffer)) + gp_pos__);  \
})

#define gp_temp_alloc_arr(self, type, len) \
	gp_temp_alloc_get(self, sizeof(type) * len)

/**
 * @brief Frees a temporary allocated memory.
 *
 * @param self A temporary allocator.
 */
#define gp_temp_alloc_free(self) \
do { \
	if (self.size > GP_ALLOCA_THRESHOLD) \
		free(self.buffer); \
} while (0)

#define gp_temp_alloc(size) GP_TEMP_ALLOC(size)

static inline void gp_temp_free(size_t size, void *ptr)
{
	if (size > GP_ALLOCA_THRESHOLD)
		free(ptr);
}

#endif /* CORE_GP_TEMP_ALLOC_H */
