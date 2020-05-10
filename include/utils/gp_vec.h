//SPDX-License-Identifier: LGPL-2.0-or-later
/*

  Copyright (C) 2020 Richard Palethorpe <richiejp@f-m.fm>
  Copyright (C) 2020 Cyril Hrubis <metan@ucw.cz>

*/

#include <stddef.h>
#include <core/gp_common.h>

#ifndef GP_VEC_H__
#define GP_VEC_H__

/* Get a pointer to the outer gp_vec struct from a pointer to the (start of)
 * its data.
 */
#define GP_VEC(ptr) (GP_CONTAINER_OF(ptr, gp_vec, payload))

/* Add an element to the end of the vec. The ptr expression must resolve to
 * a variable containing a pointer to the vec data. This variable may be
 * written to with a new pointer value. Any other copies of the pointer will
 * therefor be invalidated (see gp_vec_gap).
 */
#define GP_VEC_PUSH(ptr, elem) ({                          \
	typeof(elem) *gp_ret__;                            \
	gp_ret__ = gp_vec_insert(ptr, gp_vec_len(ptr), 1); \
	gp_ret__[gp_vec_len(gp_ret__) - 1] = elem;         \
	gp_ret__;                                          \
})

/* A growable vector */
typedef struct gp_vec {
	/* No. bytes in one element in the vec */
	size_t unit;
	/* Total capacity in units */
	size_t capacity;
	/* No. of used elements in units */
	size_t length;

	char payload[];
} gp_vec;

/*
 * Allocate a new vec and return a pointer to the data.
 */
void *gp_vec_new(size_t length, size_t unit);

/*
 * Free the vec.
 */
void gp_vec_free(void *self);

/*
 * Get the number of elements in a vec from a pointer to its data.
 */
static inline size_t gp_vec_len(const void *self)
{
	return GP_VEC(self)->length;
}

/* Insert a gap into the vec of new elements, reallocating the underlying
 * memory if more capacity is needed.
 *
 * If more capacity is required, then this will reallocate the gp_vec thus
 * invalidating *self. Therefor the caller should update any pointers it has
 * to the vec data with the return value of this function.
 *
 * Newly allocated capacity, which is not within the gap, will be set to
 * 0xff. The memory within the gap will be zeroed. If allocation fails or i is
 * invalid this will return 0. The index 'i' should be <= length.
 */
void *gp_vec_insert(void *self, size_t i, size_t length);

/*
 * Appends length elements to the end of the vector.
 */
static inline void *gp_vec_append(void *self, size_t lenght)
{
	return gp_vec_insert(self, gp_vec_len(self), lenght);
}

/*
 * Deletes a range from the vector.
 */
void *gp_vec_delete(void *self, size_t i, size_t lenght);

/*
 * Revoves length elements from the end of the vector.
 */
static inline void *gp_vec_remove(void *self, size_t length)
{
	return gp_vec_delete(self, gp_vec_len(self) - length, length);
}


#endif	/* GP_VEC_H__ */
