// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2023 Cyril Hrubis <metan@ucw.cz>
 */

/**
 * @file gp_cbuffer.h
 * @brief Circular buffer indexing and iterators.
 */

#ifndef UTILS_GP_CIRCULAR_BUFFER_H
#define UTILS_GP_CIRCULAR_BUFFER_H

#include <stddef.h>

/**
 * @brief A circular buffer indexes.
 */
typedef struct gp_cbuffer {
	/** Points one position after last inserted element */
	size_t last;
	/** The number of used buffer positions */
	size_t used;
	/** The buffer maximal size */
	size_t size;
} gp_cbuffer;

/**
 * @brief Initializes circular buffer.
 *
 * @param self A circular buffer to be initialized.
 * @param size Size of the buffer.
 */
static inline void gp_cbuffer_init(gp_cbuffer *self, size_t size)
{
	self->last = 0;
	self->used = 0;
	self->size = size;
}

/**
 * @brief Appends into circular buffer
 *
 * Only index into the buffer array is returned, any old data has to be freed
 * by the user of this API.
 *
 * @param self A circular buffer.
 * @return An index for the buffer to append.
 */
static inline size_t gp_cbuffer_append(gp_cbuffer *self)
{
	size_t old_last = self->last;

	if (self->used < self->size)
		self->used++;

	self->last = (self->last+1) % self->size;

	return old_last;
}

/**
 * @brief Returns next position in the circular buffer.
 *
 * @param self A circular buffer.
 * @param idx A current position in the buffer.
 * @return Next position in the buffer.
 */
static inline size_t gp_cbuffer_next(gp_cbuffer *self, size_t idx)
{
	return (idx+1) % self->size;
}

/**
 * @brief Returns previous position in the circular buffer.
 *
 * @param self A circular buffer.
 * @param idx A current position in the buffer.
 * @return Previous position in the buffer.
 */
static inline size_t gp_cbuffer_prev(gp_cbuffer *self, size_t idx)
{
	if (idx)
		return idx-1;

	return self->size-1;
}

/**
 * @brief Returns number of used positions in the circular buffer.
 *
 * @param self A circular buffer.
 * @return Number of used postions in the buffer.
 */
static inline size_t gp_cbuffer_used(gp_cbuffer *self)
{
	return self->used;
}

/**
 * @brief Returns index to the first element in buffer.
 *
 * Returns invalid data for empty buffer, make sure buffer is not empty with
 * gp_buffer_used() before using this function.
 *
 * @param self A circular buffer.
 * @return First used position in the buffer.
 */
static inline size_t gp_cbuffer_first(gp_cbuffer *self)
{
	size_t first;

	if (self->last >= self->used)
		first = self->last - self->used;
	else
		first = self->size - (self->used - self->last);

	return first;
}

/**
 * @brief Returns index to the last element in buffer.
 *
 * Returns invalid data for empty buffer, make sure buffer is not empty with
 * gp_buffer_used() before using this function.
 *
 * @param self A circular buffer.
 * @return Last used position in the buffer.
 */
static inline size_t gp_cbuffer_last(gp_cbuffer *self)
{
	return gp_cbuffer_prev(self, self->last);
}

/**
 * @brief An interator for loops over the circular buffer.
 */
typedef struct gp_cbuffer_iter {
	/** Index to the current position */
	size_t idx;
	/** Number of position we already iterated over */
	size_t cnt;
} gp_cbuffer_iter;

/**
 * @brief Loops over all elements in the circular buffer.
 *
 * @param self A circular buffer.
 * @param iter A circular buffer iterator variable.
 */
#define GP_CBUFFER_FOREACH(self, iter) \
	for ((iter)->idx = gp_cbuffer_first(self), (iter)->cnt = 0; \
	     (iter)->cnt < (self)->used; \
	     (iter)->idx = gp_cbuffer_next(self, (iter)->idx), (iter)->cnt++)

/**
 * @brief Loops over a range of elements in the circular buffer.
 *
 * @param self A circular buffer.
 * @param iter A circular buffer iterator variable.
 * @param skip How many element should we skip at the start.
 * @param count How many element should we iterater over.
 */
#define GP_CBUFFER_FORRANGE(self, iter, skip, count) \
	if ((self)->used > skip) \
		for ((iter)->idx = (gp_cbuffer_first(self) + skip) % (self)->size, (iter)->cnt = 0; \
		     (iter)->cnt < GP_MIN((self)->used - skip, count); \
		     (iter)->idx = gp_cbuffer_next(self, (iter)->idx), (iter)->cnt++)

/**
 * @brief Loops backwards over all elements in the circular buffer.
 *
 * @param self A circular buffer.
 * @param iter A circular buffer iterator variable.
 */
#define GP_CBUFFER_FOREACH_REV(self, iter) \
	for ((iter)->idx = gp_cbuffer_last(self), (iter)->cnt = 0; \
	     (iter)->cnt < (self)->used; \
	     (iter)->idx = gp_cbuffer_prev(self, (iter)->idx), (iter)->cnt++)

#endif /* UTILS_GP_CIRCULAR_BUFFER_H */
