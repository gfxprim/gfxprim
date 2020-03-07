// SPDX-License-Identifier: GPL-2.1-or-later
/*
 * Copyright (C) 2009-2015 Cyril Hrubis <metan@ucw.cz>
 */

/*
 * Small allocations less than or equal to GP_ALLOCA_THRESHOLD are allocated on
 * stack but the sum of stack allocated memory will never exceed GP_ALLOCA_MAX.
 */

#ifndef CORE_GP_TEMP_ALLOC_H
#define CORE_GP_TEMP_ALLOC_H

/*
 * Maximal size of request allcated using alloca.
 */
#define GP_ALLOCA_TRESHOLD 128

/*
 * Maximal size of stack allocated memory in sum.
 */
#define GP_ALLOCA_MAX 1024

typedef struct gp_temp_alloc {
	void *first;
	void *last;

	uint16_t stack_allocated;
} gp_temp_alloc;

#define GP_TEMP_ALLOC_INIT {  \
	.first = NULL,        \
	.last = NULL,         \
	.stack_allocated = 0, \
}

typedef struct gp_temp_alloc_mark {
	void *last;
	uint16_t stack_allocated;
} gp_temp_alloc_mark;

#define gp_temp_alloc(self, size) ({                               \
	size_t GP_size_ = (size);                                 \
	GP_size_ <= GP_ALLOCA_TRESHOLD &&                         \
	 (self)->stack_allocated + GP_size_ <= GP_ALLOCA_MAX    ? \
	  (self)->stack_allocated += GP_size_, alloca(GP_size_) : \
	  gp_temp_alloc_(self, size);                              \
})

#define gp_temp_alloc_arr(self, type, len) \
	gp_temp_alloc(self, sizeof(type) * (len))

void *gp_temp_alloc_(struct gp_temp_alloc *self, size_t size);

/*
 * Calling mark at given point saves the allocator state
 */
void gp_temp_alloc_save(struct gp_temp_alloc *self,
                        struct gp_temp_alloc_mark *mark);

/*
 * Frees all memory allocated allocated after the state was marked.
 */
void gp_temp_alloc_restore(struct gp_temp_alloc *self,
                           struct gp_temp_alloc_mark *mark);

/*
 * Destroys the memory pool.
 */
void gp_temp_alloc_destroy(struct gp_temp_alloc *self);

#endif /* CORE_GP_TEMP_ALLOC_H */
