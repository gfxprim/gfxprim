//SPDX-License-Identifier: LGPL-2.0-or-later

/*

   Copyright (c) 2014-2021 Cyril Hrubis <metan@ucw.cz>

 */

#include <core/gp_common.h>
#include <utils/gp_htable.h>
#include <utils/gp_htable2.h>

static size_t primes[] = {
	3,
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
	19922993,
	39845987,
	79691977,
	159383953,
	318767887,
	637535779,
	1275071537,
};

size_t gp_htable_tsize(size_t used)
{
	unsigned int order = 0;

	while (primes[order] < 2*used)
		order++;

	return primes[order];
}

int gp_htable_init(gp_htable *self, unsigned int order, int flags)
{
	order = GP_MIN(order, GP_ARRAY_SIZE(primes)-1);

	size_t size = primes[order] * sizeof(struct gp_htable_rec);
	struct gp_htable_rec *recs = malloc(size);

	if (!recs) {
		GP_WARN("Malloc failed :-(");
		return 1;
	}

	memset(recs, 0, size);
	self->size = primes[order];
	self->used = 0;
	self->flags = flags;
	self->recs = recs;

	return 0;
}

gp_htable *gp_htable_new(unsigned int order, int flags)
{
	gp_htable *table = malloc(sizeof(gp_htable));

	GP_DEBUG(1, "Allocating hash table order %u", order);

	if (!table) {
		GP_WARN("Malloc failed :-(");
		return NULL;
	}

	if (gp_htable_init(table, order, flags)) {
		free(table);
		return NULL;
	}

	table->flags |= GP_HTABLE_FREE_SELF;

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
			free(self->recs[i].key);
	}

	free(self->recs);

	if (self->flags & GP_HTABLE_FREE_SELF)
		free(self);
}

void gp_htable_put(gp_htable *self, void *val, char *key)
{
	if (self->flags & GP_HTABLE_COPY_KEY)
		key = strdup(key);

	if (!key) {
		GP_WARN("Malloc failed!");
		return;
	}

	gp_htable_put2(self, gp_htable_strhash, val, key);
}

void *gp_htable_get(gp_htable *self, const char *key)
{
	return gp_htable_get2(self, gp_htable_strhash, gp_htable_strcmp, key);
}

void *gp_htable_rem(gp_htable *self, const char *key)
{
	return gp_htable_rem2(self, gp_htable_strhash, gp_htable_strcmp, key);
}
