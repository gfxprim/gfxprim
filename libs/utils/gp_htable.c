//SPDX-License-Identifier: LGPL-2.0-or-later

/*

   Copyright (c) 2014-2019 Cyril Hrubis <metan@ucw.cz>

 */

#include <stdlib.h>
#include <string.h>
#include <core/gp_common.h>
#include <core/gp_debug.h>
#include <utils/gp_htable.h>

static unsigned int primes[] = {
	7,
	19,
	37,
	79,
	151,
	307,
	607,
	1217,
	2437,
	4871,
	9733,
	19469,
	38917,
	77839,
	155653,
	311299,
	622603,
	1245187,
	2490377,
	4980749,
	9961487,
};

static unsigned int hash(const char *key, unsigned int size)
{
	unsigned int h = 0;

	while (*key)
		h = (h * 151 + *key++) % size;

	return h;
}

typedef struct {
	char *key;
	void *val;
} htable_elem;

struct gp_htable {
	htable_elem *elems;
	unsigned int size;
	unsigned int used;
	int flags;
};

gp_htable *gp_htable_new(unsigned int order, int flags)
{
	order = GP_MIN(order, GP_ARRAY_SIZE(primes));

	size_t size = primes[order] * sizeof(htable_elem);
	htable_elem *elems = malloc(size);
	gp_htable *table = malloc(sizeof(gp_htable));


	GP_DEBUG(1, "Allocating hash table order %u", order);

	if (!table || !elems) {
		free(table);
		free(elems);
		GP_WARN("Malloc failed :-(");
		return NULL;
	}

	memset(elems, 0, size);
	table->size = primes[order];
	table->used = 0;
	table->flags = flags;
	table->elems = elems;

	return table;
}

void gp_htable_free(gp_htable *self)
{
	if (!self)
		return;

	if (self->flags & GP_HTABLE_COPY_KEY ||
	    self->flags & GP_HTABLE_FREE_KEY) {
		unsigned int i;

		for (i = 0; i < self->size; i++)
			free(self->elems[i].key);
	}

	free(self->elems);
	free(self);
}

static void put(htable_elem *elems, unsigned int size,
                void *val, char *key)
{
	unsigned int h = hash(key, size);

	while (elems[h].val)
		h = (h+1) % size;

	elems[h].val = val;
	elems[h].key = key;
}

static void rehash(gp_htable *self)
{
	unsigned int i, order = 0;

	//TODO: Abort on overflow!
	while (self->size >= primes[order])
		order++;

	unsigned int size = primes[order];

	GP_DEBUG(1, "Rehashing from %u to %u", self->size, size);

	htable_elem *elems = malloc(sizeof(htable_elem) * size);

	if (!elems) {
		GP_WARN("Malloc failed :-(");
		return;
	}

	memset(elems, 0, sizeof(htable_elem) * size);

	for (i = 0; i < self->size; i++) {
		if (!self->elems[i].val)
			continue;

		put(elems, size, self->elems[i].val, self->elems[i].key);
	}

	free(self->elems);
	self->elems = elems;
	self->size = size;
}

void gp_htable_put(gp_htable *self, void *val, char *key)
{
	if (self->flags & GP_HTABLE_COPY_KEY)
		key = strdup(key);

	if (!key) {
		GP_WARN("Malloc failed :-(");
		return;
	}

	if (++self->used >= self->size / 2)
		rehash(self);

	put(self->elems, self->size, val, key);
}

void *gp_htable_get(gp_htable *self, const char *key)
{
	if (!self)
		return NULL;

	unsigned int h = hash(key, self->size);

	while (self->elems[h].val) {
		if (!strcmp(self->elems[h].key, key))
			return self->elems[h].val;
		h = (h + 1) % self->size;
	}

	return NULL;
}
