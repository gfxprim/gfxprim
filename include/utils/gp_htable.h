//SPDX-License-Identifier: LGPL-2.0-or-later

/*

   Copyright (c) 2014-2021 Cyril Hrubis <metan@ucw.cz>

 */

/**
 * @file gp_htable.h
 * @brief Simple hash table implementation.
 */

#ifndef GP_HTABLE_H
#define GP_HTABLE_H

#include <stddef.h>
#include <string.h>
#include <utils/gp_types.h>

/**
 * @brief Flags to change how to deal with the hash table string keys.
 */
enum gp_htable_flags {
	/** The key is copied on insert. */
	GP_HTABLE_COPY_KEY = 0x01,
	/** The key is freed on removal. */
	GP_HTABLE_FREE_KEY = 0x02,
	/** The gp_htable is freed at the end of gp_htable_free() */
	GP_HTABLE_FREE_SELF = 0x04,
};

/** @brief A hash table record. */
struct gp_htable_rec {
	void *key;
	void *val;
};

/** @brief A hash table. */
struct gp_htable {
	/** @brief Array for the hash table records. */
	struct gp_htable_rec *recs;
	/** @brief Hash table record array size. */
        size_t size;
	/** @brief Number of used slots in the hash table */
        size_t used;
        /** @brief Flags. */
	enum gp_htable_flags flags;
};

/** Hash table iterator. */
#define GP_HTABLE_FOREACH(table, var) \
	for (struct gp_htable_rec *var = (table)->recs; var < &((table)->recs[(table)->size]); var++) \
		if (var->key)

/**
 * @brief Allocates a hash table.
 *
 * @param order Hint of log2(size) for expected number of hash elements, if unsure pass 0.
 * @param flags See enum gp_htable_flags.
 *
 * Note that GP_HTABLE_FREE_SELF is added automatically to the flags so that
 * the resulting table is freed on gp_htable_free().
 *
 * @return Newly allocated hash table or NULL in a case of a malloc failure.
 */
gp_htable *gp_htable_new(unsigned int order, int flags);

/**
 * @brief Initializes an hash table embedded in a different structure.
 *
 * @param self A pointer to an hash table to be initialized.
 * @param order Hint of log2(size) for expected number of hash elements, if unsure pass 0.
 * @param flags See enum gp_htable_flags.
 *
 * @return Zero on success, non-zero on allocation failure.
 */
int gp_htable_init(gp_htable *self, unsigned int order, int flags);

/**
 * @brief Returns the number of keys in hash table.
 *
 * @param self A hash table.
 *
 * @return The number of keys in hash table.
 */
static inline size_t gp_htable_keys(gp_htable *self)
{
	return self->used;
}

/**
 * @brief Frees a hash table.
 *
 * @param self The table to be freed.
 */
void gp_htable_free(gp_htable *self);

/**
 * @brief A string hashing function.
 *
 * @param key A string.
 * @param htable_size A hash table size.
 */
static inline size_t gp_htable_strhash(const void *key, size_t htable_size)
{
        unsigned int h = 0;
	const char *str = key;

        while (*str)
                h = (h * 151 + *str++) % htable_size;

        return h;
}

/**
 * @brief A string matching function.
 *
 * @param key1 A string.
 * @param key2 A string.
 *
 * @return Non-zero if keys are equal.
 */
static inline int gp_htable_strcmp(const void *key1, const void *key2)
{
	return !strcmp(key1, key2);
}

/**
 * @brief Adds a pointer to a hash table.
 *
 * @param self Hash table.
 * @param val A value.
 * @param key A string key.
 */
void gp_htable_put(gp_htable *self, void *val, char *key);

/**
 * @brief Search for an element given a string key.
 *
 * @param self A hash table.
 * @param key A string key.
 *
 * @return A value if found or NULL.
 */
void *gp_htable_get(gp_htable *self, const char *key);

/**
 * @brief Removes an entry from a hash table.
 *
 * @param self A Hash table.
 * @param key A string key.
 *
 * @return A value for removed key or NULL if not found.
 */
void *gp_htable_rem(gp_htable *self, const char *key);

#endif /* GP_HTABLE_H */
