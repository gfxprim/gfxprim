//SPDX-License-Identifier: LGPL-2.0-or-later
/*

  Copyright (C) 2020 Richard Palethorpe <richiejp@f-m.fm>
  Copyright (C) 2020 Cyril Hrubis <metan@ucw.cz>

*/

/**
 * @file gp_vec.h
 * @brief Vector a growable array.
 */

#include <stddef.h>
#include <core/gp_common.h>
#include <core/gp_compiler.h>

#ifndef GP_VEC_H
#define GP_VEC_H

/**
 * @brief Converts a vector pointer into the header i.e. struct gp_vec.
 *
 * Returns a pointer to the outer gp_vec struct from a pointer to the (start of)
 * its data.
 */
#define GP_VEC(ptr) (GP_CONTAINER_OF(ptr, gp_vec, payload))

/** @brief A growable vector */
typedef struct gp_vec {
	/** No. bytes in one element in the vec */
	size_t unit;
	/** Total capacity in units */
	size_t capacity;
	/** No. of used elements in units */
	size_t length;
	/** Start of the buffer data */
	char payload[];
} gp_vec;

/**
 * @brief Expands vector length; does not touch vector data.
 *
 * This is internal rutine, handle with care!
 *
 * @param self Pointer to the vector structure.
 * @param len How many units should be added to vector length.
 *
 * @return A pointer to a vector data.
 */
GP_WUR gp_vec *gp_vec_expand_(gp_vec *self, size_t len);

/**
 * @brief Shrinks vector length; does not touch vector data.
 *
 * This is internal rutine, handle with care!
 *
 * @param self Pointer to the vector structure.
 * @param len How many units should be removed from vector length.
 *
 * @return A pointer to a vector data.
 */
GP_WUR gp_vec *gp_vec_shrink_(gp_vec *self, size_t len);

/**
 * @brief Allocates a new vector.
 *
 * @param len An initial lenght of the vector.
 * @param unit A size of a vector element.
 *
 * @return Returns a pointer to the vector data.
 */
GP_WUR void *gp_vec_new(size_t len, size_t unit);

/**
 * @brief Creates a duplicate of a vector.
 *
 * Creates a new vector with the exact same data.
 *
 * @param self Vector to be duplicated.
 * @return Returns a pointer to a new vector.
 */
GP_WUR void *gp_vec_dup(void *self);

/**
 * @brief Frees the vector.
 *
 * @param self A vector. The call is no-op on NULL vector.
 */
void gp_vec_free(void *self);

/**
 * @brief Resize vector.
 *
 * Returns a pointer to the vector, possibly a different from the
 * previous one. May return NULL if vector grows and underlying call to
 * realloc() has failed.
 *
 * @param self A vector.
 * @param length A new vector lenght.
 * @return A (new) pointer to the vector.
 */
GP_WUR void *gp_vec_resize(void *self, size_t len);

/**
 * @brief Returns vector lenght.
 *
 * @param self A vector.
 * @return A number of elements in the vector.
 */
static inline size_t gp_vec_len(const void *self)
{
	return GP_VEC(self)->length;
}

/**
 * @brief Returns vector unit.
 *
 * @param self A vector.
 * @return A vector unit, i.e. size of a vector element.
 */
static inline size_t gp_vec_unit(const void *self)
{
	return GP_VEC(self)->unit;
}

/**
 * @brief Insert a gap into the vec of new elements, reallocating the
 *        underlying memory if more capacity is needed.
 *
 * If more capacity is required, then this will reallocate the gp_vec thus
 * invalidating *self. Therefor the caller should update any pointers it has
 * to the vec data with the return value of this function.
 *
 * Newly allocated capacity, which is not within the gap, will be set to
 * 0xff. The memory within the gap will be zeroed. If allocation fails or i is
 * invalid this will return 0. The offset 'off' should be <= length.
 *
 * Returns a pointer to the vector, possibly a different from the
 * previous one. May return NULL if underlying call to realloc() has
 * failed or if off is outside of the vector.
 *
 * @param self A vector.
 * @param off An offset in the vector.
 * @param length A number of elements to instert.
 *
 * @return A (new) pointer to the vector.
 */
GP_WUR void *gp_vec_ins(void *self, size_t off, size_t len);

/**
 * @brief Expands vector by length elements at the end of the vector.
 *
 * @param self A vector.
 * @param length A number of elements to append.
 *
 * Returns a pointer to the vector, possibly a different from the
 * previous one. May return NULL if underlying call to realloc() has
 * failed.
 *
 * @return A (new) pointer to the vector.
 */
GP_WUR void *gp_vec_expand(void *self, size_t len);

/**
 * @brief Deletes a range from the vector.
 *
 * @param self A vector.
 * @param off An offset in the vector.
 * @param length A number of elements to delete.
 *
 * Returns a pointer to the vector, possibly a different from the
 * previous one. May return NULL if block is outside of the vector.
 *
 * @return A (new) pointer to the vector.
 */
GP_WUR void *gp_vec_del(void *self, size_t off, size_t len);

/**
 * @brief Shrinks vector by length elements at the end of the vector.
 *
 * Returns a pointer to the vector, possibly a different from the previous one.
 *
 * @param self A vector.
 * @param length A number of elemements to remove.
 * @return A (new) pointer to the vector.
 */
GP_WUR void *gp_vec_shrink(void *self, size_t len);

/**
 * @brief Moves last object in vector to idx and shrinks the vector by one.
 *
 * This is fast operation to remove an object from the middle of the vector by
 * replacing it by last element in vector and then shrinking it.
 *
 * @param vec A vector.
 * @param idx An index into the vector to be removed.
 *
 * @return A (new) pointer to the vector or NULL in a case of a failure.
 */
GP_WUR void *gp_vec_move_shrink(void *self, size_t idx);

/**
 * @brief A vector iterator.
 *
 * A ready made iterator for the vector data.
 *
 * @param self A vector.
 * @param type A C type of the vector data.
 * @param iterator The iterator variable name.
 *
 * For a vector of integers it's used as:
 * @code
 * GP_VEC_FOREACH(int_vec, int, i)
 *         printf("%i\n", *i);
 * @endcode
 *
 * And for a vector of strings:
 * @code
 * GP_VEC_FOREACH(strs, char*, str)
 *         printf("%s\n", *str);
 * @endcode
 */
#define GP_VEC_FOREACH(self, type, iterator) \
	for (type *iterator = (self); iterator < (self) + gp_vec_len(self); iterator++)

/**
 * @brief Appends single element to a vector.
 *
 * @param vec A vector.
 * @param val A value to be appended.
 * @return NULL on a failure.
 */
#define GP_VEC_APPEND(vec, val) ({ \
	typeof(vec) gp_ret__ = gp_vec_expand(vec, 1); \
	if (gp_ret__) { \
		gp_ret__[gp_vec_len(gp_ret__)-1] = val; \
		vec = gp_ret__; \
	} \
	gp_ret__; \
})

#endif	/* GP_VEC_H */
