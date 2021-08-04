//SPDX-License-Identifier: LGPL-2.0-or-later

/*

   Simple hash table implementation - generic part.

   These functions are basic building blocks for a hash table.

   Copyright (c) 2014-2021 Cyril Hrubis <metan@ucw.cz>

 */

#ifndef GP_HTABLE2_H
#define GP_HTABLE2_H

#include <stddef.h>
#include <string.h>
#include <core/gp_debug.h>
#include <utils/gp_htable.h>

/* Returns new size for table based on number of used records. */
size_t gp_htable_tsize(size_t used);

static inline void gp_htable_put_(struct gp_htable_rec *recs,
                                  size_t (*hash)(const void *key, size_t htable_size),
                                  size_t htable_size,
                                  void *val, char *key)
{
	unsigned int h = hash(key, htable_size);

	while (recs[h].key)
		h = (h+1) % htable_size;

	recs[h].val = val;
	recs[h].key = key;
}

static inline void gp_htable_rehash(gp_htable *self, size_t new_size,
                                    size_t (*hash)(const void *key, size_t htable_size))
{
	size_t i;

	GP_DEBUG(1, "Rehashing from %zu to %zu", self->size, new_size);

	struct gp_htable_rec *recs = malloc(sizeof(struct gp_htable_rec) * new_size);

	if (!recs) {
		GP_WARN("Malloc failed :-(");
		return;
	}

	memset(recs, 0, sizeof(struct gp_htable_rec) * new_size);

	for (i = 0; i < self->size; i++) {
		if (!self->recs[i].key)
			continue;

		gp_htable_put_(recs, hash, new_size, self->recs[i].val, self->recs[i].key);
	}

	free(self->recs);
	self->recs = recs;
	self->size = new_size;
}

static inline void gp_htable_put2(gp_htable *self,
                                  size_t (*hash)(const void *key, size_t htable_size),
                                  void *val, void *key)
{
	if (++self->used > self->size/2)
		gp_htable_rehash(self, gp_htable_tsize(self->used), hash);

	gp_htable_put_(self->recs, hash, self->size, val, key);
}

static inline void *gp_htable_get2(gp_htable *self,
                                   size_t (*hash)(const void *key, size_t htable_size),
                                   int (*cmp)(const void *key1, const void *key2),
                                   const void *key)
{
	if (!self)
		return NULL;

	size_t h = hash(key, self->size);

	while (self->recs[h].key) {
		if (cmp(self->recs[h].key, key))
			return self->recs[h].val;
		h = (h+1) % self->size;
	}

	return NULL;
}

static inline void *gp_htable_rem2_(gp_htable *self,
                                    size_t (*hash)(const void *key, size_t htable_size),
                                    size_t h)
{
	void *ret;

	if (self->flags & GP_HTABLE_FREE_KEY)
		free(self->recs[h].key);

	ret = self->recs[h].val;

	self->recs[h].key = NULL;
	self->recs[h].val = NULL;

	if (--self->used < self->size/8) {
		gp_htable_rehash(self, gp_htable_tsize(self->used), hash);
		return ret;
	}

	size_t i = h;
	size_t j = h;

	for (;;) {
		i = (i+1)%self->size;

		if (!self->recs[i].key)
			break;

		h = hash(self->recs[i].key, self->size);

		/* record at i can't be moved to the empty slot j */
		if (i >= h && h > j)
			continue;

		/* the same but i has overflown over self->size */
		if (i < j && h <= i)
			continue;

		if (i < j && h > j)
			continue;

		self->recs[j] = self->recs[i];

		self->recs[i].key = NULL;
		self->recs[i].val = NULL;

		j = i;
	}

	return ret;
}

static inline void *gp_htable_rem2(gp_htable *self,
                                   size_t (*hash)(const void *key, size_t htable_size),
                                   int (*cmp)(const void *key1, const void *key2),
                                   const void *key)
{
	size_t h = hash(key, self->size);

	while (self->recs[h].key) {
		if (cmp(self->recs[h].key, key))
			return gp_htable_rem2_(self, hash, h);

		h = (h+1) % self->size;
	}

	return NULL;
}

/*
 * Iterates over all hash table records, calls trim() on each once. If trim()
 * returns non-zero record is removed from the table.
 *
 * We cannot remove elements from the table in the middle of the loop, since
 * removal shuffles them around the table. Instead we build a list of to be
 * deleted elements reusing the val pointer in record and do the removal once
 * we evaluated all records.
 */
static inline void gp_htable_trim2(gp_htable *self,
                                   size_t (*hash)(const void *key, size_t htable_size),
                                   int (*cmp)(const void *key1, const void *key2),
				   int (*trim)(void *val),
				   void (*free_val)(void *val))
{
	size_t i = 0;
	void *key = NULL;

	for (i = 0; i < self->size; i++) {
		if (!self->recs[i].key)
			continue;

		if (!trim(self->recs[i].val))
			continue;

		if (free_val)
			free_val(self->recs[i].val);

		self->recs[i].val = key;
		key = self->recs[i].key;
	}

	while (key)
		key = gp_htable_rem2(self, hash, cmp, key);
}

#endif /* GP_HTABLE2_H */
