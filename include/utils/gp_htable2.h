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

static inline void *gp_htable_rem2(gp_htable *self,
                                   size_t (*hash)(const void *key, size_t htable_size),
                                   int (*cmp)(const void *key1, const void *key2),
                                   const void *key)
{
	size_t h = hash(key, self->size);
	void *ret;

	while (self->recs[h].key) {
		if (cmp(self->recs[h].key, key)) {
			if (self->flags & GP_HTABLE_FREE_KEY)
				free(self->recs[h].key);

			ret = self->recs[h].val;

			self->recs[h].key = NULL;
			self->recs[h].val = NULL;

			if (--self->used < self->size/8) {
				gp_htable_rehash(self, gp_htable_tsize(self->used), hash);
			} else {
				for (;;) {
					size_t e = h;

					h = (h+1) % self->size;

					if (!self->recs[h].key)
						break;

					if (h == hash(self->recs[h].key, self->size))
						break;

					self->recs[e] = self->recs[h];

					self->recs[h].key = NULL;
					self->recs[h].val = NULL;
				}
			}

			return ret;
		}

		h = (h+1) % self->size;
	}

	return NULL;
}

#endif /* GP_HTABLE2_H */
