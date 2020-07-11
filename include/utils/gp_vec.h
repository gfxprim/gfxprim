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

/* @brief Expands vector length; does not touch vector data.
 *
 * This is internal rutine, handle with care!
 *
 * @self   Pointer to the vector structure.
 * @lenght How many units should be added to vector length.
 *
 * @return A pointer to a vector data.
 */
void *gp_vec_expand(gp_vec *self, size_t length);

/* @brief Shirnks vector length; does not touch vector data.
 *
 * This is internal rutine, handle with care!
 *
 * @self   Pointer to the vector structure.
 * @lenght How many units should be removed from vector length.
 *
 * @return A pointer to a vector data.
 */
void *gp_vec_shrink(gp_vec *self, size_t length);

/*
 * @brief  Allocates a new vector.
 *
 * @lenght A lenght of the vector.
 * @unit   A size of a vector element.
 *
 * @return Returns a pointer to the vector data.
 */
void *gp_vec_new(size_t length, size_t unit);

/*
 * @brief Frees the vector.
 *
 * @self A vector.
 */
void gp_vec_free(void *self);

/*
 * @brief Resize vector.
 *
 * @self   A vector.
 * @length A new vector lenght.
 *
 * @return Returns a pointer to the vector, possibly a different from the
 *         previous one. May return NULL if vector grows and underlying call to
 *         realloc() has failed.
 */
void *gp_vec_resize(void *self, size_t length);

/*
 * @brief Returns vector lenght.
 *
 * @self A vector.
 *
 * @return A number of elements in the vector.
 */
static inline size_t gp_vec_len(const void *self)
{
	return GP_VEC(self)->length;
}

/* @brief Insert a gap into the vec of new elements, reallocating the
 *        underlying memory if more capacity is needed.
 *
 * @self A vector.
 * @off  An offset in the vector.
 * @length A number of elements to instert.
 *
 * If more capacity is required, then this will reallocate the gp_vec thus
 * invalidating *self. Therefor the caller should update any pointers it has
 * to the vec data with the return value of this function.
 *
 * Newly allocated capacity, which is not within the gap, will be set to
 * 0xff. The memory within the gap will be zeroed. If allocation fails or i is
 * invalid this will return 0. The offset 'off' should be <= length.
 *
 * @return Returns a pointer to the vector, possibly a different from the
 *         previous one. May return NULL if underlying call to realloc() has
 *         failed or if off is outside of the vector.
 */
void *gp_vec_insert(void *self, size_t off, size_t length);

/*
 * @breif Appends length elements to the end of the vector.
 *
 * @self A vector.
 * @length A number of elements to append.
 *
 * Calls gp_vec_insert() with an offset equal to vector length.
 *
 * @return Returns a pointer to the vector, possibly a different from the
 *         previous one. May return NULL if underlying call to realloc() has
 *         failed.
 */
static inline void *gp_vec_append(void *self, size_t lenght)
{
	return gp_vec_insert(self, gp_vec_len(self), lenght);
}

/*
 * @brief Deletes a range from the vector.
 *
 * @self A vector.
 * @off An offset in the vector.
 * @length A number of elements to delete.
 *
 * @return Returns a pointer to the vector, possibly a different from the
 *         previous one. May return NULL if off is outside of the vector.
 */
void *gp_vec_delete(void *self, size_t off, size_t lenght);

/*
 * @brief Removes length elements from the end of the vector.
 *
 * @self A vector.
 * @length A number of elemements to remove.
 *
 * @return Returns a pointer to the vector, possibly a different from the
 *         previous one.
 */
static inline void *gp_vec_remove(void *self, size_t length)
{
	return gp_vec_delete(self, gp_vec_len(self) - length, length);
}

#endif	/* GP_VEC_H__ */
