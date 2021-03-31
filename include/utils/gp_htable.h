//SPDX-License-Identifier: LGPL-2.0-or-later

/*

   Simple hash table implementation.

   Copyright (c) 2014-2021 Cyril Hrubis <metan@ucw.cz>

 */

#ifndef GP_HTABLE_H
#define GP_HTABLE_H

#include <utils/gp_types.h>

enum gp_htable_flags {
	GP_HTABLE_COPY_KEY = 0x01,
	GP_HTABLE_FREE_KEY = 0x02,
};

/**
 * @brief Allocates a hash table.
 *
 * @param order Hint of log2(size) for expected number of hash elements, if unsure pass 0.
 * @param flags See gp_htable_flags.
 * @return Newly allocated hash table or NULL in a case of a failure.
 */
gp_htable *gp_htable_new(unsigned int order, int flags);

/**
 * @brief Frees a hash table.
 *
 * @param table The table to be freed.
 */
void gp_htable_free(gp_htable *self);

/**
 * @brief Adds a pointer to a hash table.
 *
 * @param Hash table.
 * @param ptr A value.
 * @param key A string key.
 */
void gp_htable_put(gp_htable *self, void *val, char *key);

/**
 * @brief Search for an element given a string key.
 *
 * @param Hash table.
 * @param key A string key.
 * @return A value pointer if found or NULL.
 */
void *gp_htable_get(gp_htable *self, const char *key);

#endif /* GP_HTABLE_H */
