//SPDX-License-Identifier: LGPL-2.0-or-later
/*

  Copyright (C) 2020 Richard Palethorpe <richiejp@f-m.fm>
  Copyright (C) 2020 Cyril Hrubis <metan@ucw.cz>

*/

#include <string.h>
#include <core/gp_common.h>
#include <core/gp_debug.h>
#include <utils/gp_vec.h>

static size_t new_capacity(size_t length, size_t cur_capacity)
{
	size_t capacity = GP_MAX(2u, cur_capacity);

	while (length >= capacity)
		capacity *= 2;

	return capacity;
}

static void *expand(gp_vec *self, size_t length)
{
	size_t capacity;

	capacity = new_capacity(self->length + length, self->capacity);

	if (capacity == self->capacity)
		goto ret;

	self = realloc(self, sizeof(gp_vec) + capacity * self->unit);
	if (!self)
		return NULL;

	memset(self->payload + self->length * self->unit, 0xff,
	       (capacity - self->length) * self->unit);

	self->capacity = capacity;
ret:
	self->length += length;

	return self;
}

void *gp_vec_new(size_t length, size_t unit)
{
	size_t capacity = new_capacity(length, 0);
	gp_vec *self = malloc(sizeof(gp_vec) + capacity	* unit);

	if (!self)
		return NULL;

	self->unit = unit;
	self->capacity = capacity;
	self->length = length;

	memset(self->payload, 0, length * unit);
	memset(self->payload + length * unit, 0xff, (capacity - length) * unit);

	return (void *)self->payload;
}

void gp_vec_free(void *self)
{
	if (!self)
		return;

	gp_vec *vec = GP_VEC(self);

	free(vec);
}

void *gp_vec_insert(void *self, size_t i, size_t length)
{
	gp_vec *vec = GP_VEC(self);

	if (i > vec->length) {
		GP_WARN("Index (%zu) out of vector %p size %zu",
			i, self, vec->length);
		return NULL;
	}

	vec = expand(vec, length);
	if (!vec)
		return NULL;

	if (i >= vec->length - length)
		goto out;

	memmove(vec->payload + (i + length) * vec->unit,
		vec->payload + i * vec->unit,
		(vec->length - length - i) * vec->unit);

out:
	memset(vec->payload + i * vec->unit, 0, length * vec->unit);
	return (void *)vec->payload;
}

void *gp_vec_delete(void *self, size_t i, size_t length)
{
	gp_vec *vec = GP_VEC(self);

	if (length == 0)
		return self;

	if (i + length > vec->length) {
		GP_WARN("Block (%zu-%zu) out of vector %p size %zu",
		        i, length, self, vec->length);
		return NULL;
	}

	memmove(vec->payload + i * vec->unit,
		vec->payload + (i + length) * vec->unit,
		(vec->length - length - i) * vec->unit);

	memset(vec->payload + (vec->length - length) * vec->unit,
	       0xff, length * vec->unit);

	vec->length -= length;

	size_t capacity = vec->capacity;

	while ((capacity >> 1) > vec->length && (capacity >> 1) > 2u)
		capacity >>= 1;

	if (capacity == vec->capacity)
		goto ret;

	gp_vec *rvec = realloc(vec, sizeof(*vec) + capacity * vec->unit);

	if (rvec) {
		vec = rvec;
		vec->capacity = capacity;
	}

ret:
	return (void *)vec->payload;
}
