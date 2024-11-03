//SPDX-License-Identifier: LGPL-2.0-or-later
/*

  Copyright (C) 2020 Richard Palethorpe <richiejp@f-m.fm>
  Copyright (C) 2020-2021 Cyril Hrubis <metan@ucw.cz>

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

gp_vec *gp_vec_expand_(gp_vec *self, size_t length)
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

void *gp_vec_expand(void *self, size_t length)
{
	gp_vec *ret;

	ret = gp_vec_expand_(GP_VEC(self), length);
	if (!ret)
		return NULL;

	return (void*)ret->payload;
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

void *gp_vec_dup(void *self)
{
	gp_vec *vec = GP_VEC(self);

	void *copy = gp_vec_new(vec->length, vec->unit);
	if (!copy)
		return NULL;

	memcpy(copy, self, vec->length * vec->unit);

	return copy;
}

void gp_vec_free(void *self)
{
	if (!self)
		return;

	gp_vec *vec = GP_VEC(self);

	free(vec);
}

void *gp_vec_resize(void *self, size_t length)
{
	gp_vec *vec = GP_VEC(self);

	if (vec->length == length)
		return self;

	if (vec->length < length)
		return gp_vec_expand(self, length - vec->length);

	return gp_vec_shrink(self, vec->length - length);
}

void *gp_vec_ins(void *self, size_t i, size_t length)
{
	gp_vec *vec = GP_VEC(self);

	if (i > vec->length) {
		GP_WARN("Index (%zu) out of vector %p size %zu",
			i, self, vec->length);
		return NULL;
	}

	vec = gp_vec_expand_(vec, length);
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

gp_vec *gp_vec_shrink_(gp_vec *vec, size_t length)
{
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
	return vec;
}

void *gp_vec_shrink(void *self, size_t length)
{
	gp_vec *vec = GP_VEC(self);

	if (length > vec->length) {
		GP_WARN("Vector too short %zu shrink %zu",
			vec->length, length);
		return NULL;
	}

	vec = gp_vec_shrink_(vec, length);

	return (void*)vec->payload;
}

GP_WUR void *gp_vec_move_shrink(void *self, size_t idx)
{
	gp_vec *vec = GP_VEC(self);

	if (idx >= vec->length) {
		GP_WARN("Index out of range %zu length %zu",
			idx, vec->length);
		return NULL;
	}

	memmove(vec->payload + idx * vec->unit,
	        vec->payload + (vec->length-1) * vec->unit,
	        vec->unit);

	vec = gp_vec_shrink_(vec, 1);

	return (void*)vec->payload;
}

void *gp_vec_del(void *self, size_t i, size_t length)
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

	vec = gp_vec_shrink_(vec, length);

	return (void*)vec->payload;
}
