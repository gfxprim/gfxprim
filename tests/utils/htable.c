// SPDX-License-Identifier: GPL-2.1-or-later
/*

  Copyright (C) 2007-2021 Cyril Hrubis <metan@ucw.cz>

 */

#include <stdint.h>
#include <utils/gp_htable.h>
#include <utils/gp_htable2.h>

#include "tst_test.h"

static int htable_test(void)
{
	gp_htable *table = gp_htable_new(0, GP_HTABLE_COPY_KEY | GP_HTABLE_FREE_KEY);
	void *val;

	if (!table) {
		tst_msg("Malloc failed :-(");
		return TST_FAILED;
	}

	gp_htable_put(table, (void*)(intptr_t)42, "42");

	if (gp_htable_get(table, "not-a-key")) {
		tst_msg("Got hit for non-existent key");
		return TST_FAILED;
	} else {
		tst_msg("Got NULL for non-existent key");
	}

	val = gp_htable_get(table, "42");
	if ((intptr_t)val != 42) {
		tst_msg("Got wrong value for existing key %li", (intptr_t)val);
		return TST_FAILED;
	} else {
		tst_msg("Got correct value for existing key");
	}

	val = gp_htable_rem(table, "not-a-key");
	if (val) {
		tst_msg("Got non-zero pointer on non-existent removal");
		return TST_FAILED;
	} else {
		tst_msg("Got NULL for non-existing key removal");
	}

	val = gp_htable_rem(table, "42");
	if ((intptr_t)val != 42) {
		tst_msg("Got wrong value when removing key %li", (intptr_t)val);
		return TST_FAILED;
	} else {
		tst_msg("Got correct value for key removal");
	}

	if (gp_htable_get(table, "42")) {
		tst_msg("Got hit for removed key");
		return TST_FAILED;
	} else {
		tst_msg("Got NULL on lookup for removed key");
	}

	gp_htable_free(table);

	return TST_PASSED;
}

/* Stupid hash function to ensure collisions */
static size_t stupid_hash(const void *key, size_t size, void *priv)
{
	const char *str = key;

	(void) priv;

	return str[0] % size;
}

static int lookup_success(gp_htable *table, const char *key, const char *val)
{
	const char *rval;

	rval = gp_htable_get2(table, stupid_hash, gp_htable_strcmp, key);
	if (!rval) {
		tst_msg("Failed to find value for '%s'", key);
		return TST_FAILED;
	}

	if (strcmp(rval, val)) {
		tst_msg("Wrong value for '%s' = %s expected %s", key, rval, val);
		return TST_FAILED;
	}

	tst_msg("Got correct value for '%s' = %s", key, val);

	return TST_PASSED;
}

static int lookup_failure(gp_htable *table, const char *key)
{
	const unsigned char *rval;

	rval = gp_htable_get2(table, stupid_hash, gp_htable_strcmp, key);
	if (rval) {
		tst_msg("Found value for '%s'", key);
		return TST_FAILED;
	}

	tst_msg("Key '%s' not found", key);

	return TST_PASSED;
}

static int collision_test(void)
{
	gp_htable *table = gp_htable_new(0, 0);
	const char *val;

	if (!table) {
		tst_msg("Malloc failed :-(");
		return TST_FAILED;
	}

	tst_msg("Inserting 'abc-key' size=%zu", table->size);
	gp_htable_put2(table, stupid_hash, "abc", "abc-key");
	tst_msg("Inserting 'aaa-key' size=%zu", table->size);
	gp_htable_put2(table, stupid_hash, "aaa", "aaa-key");
	tst_msg("Inserting 'ccc-key' size=%zu", table->size);
	gp_htable_put2(table, stupid_hash, "ccc", "ccc-key");

	if (lookup_success(table, "abc-key", "abc"))
		return TST_FAILED;

	if (lookup_success(table, "aaa-key", "aaa"))
		return TST_FAILED;

	if (lookup_success(table, "ccc-key", "ccc"))
		return TST_FAILED;

	val = gp_htable_rem2(table, stupid_hash, gp_htable_strcmp, "abc-key");
	if (!val || strcmp(val, "abc")) {
		tst_msg("Wrong value for removal of 'abc-key' = %s", val);
		return TST_FAILED;
	} else {
		tst_msg("Removed key 'abc-key'");
	}

	if (lookup_failure(table, "abc-key"))
		return TST_FAILED;

	if (lookup_success(table, "aaa-key", "aaa"))
		return TST_FAILED;

	if (lookup_success(table, "ccc-key", "ccc"))
		return TST_FAILED;

	tst_msg("Reinserting 'abc-key' size=%zu", table->size);
	gp_htable_put2(table, stupid_hash, "abc", "abc-key");
	tst_msg("Inserting 'abb-key' size=%zu", table->size);
	gp_htable_put2(table, stupid_hash, "abb", "abb-key");

	tst_msg("Inserting 'abc-key', 'abb-key'");

	if (lookup_success(table, "aaa-key", "aaa"))
		return TST_FAILED;

	if (lookup_success(table, "ccc-key", "ccc"))
		return TST_FAILED;

	if (lookup_success(table, "abc-key", "abc"))
		return TST_FAILED;

	if (lookup_success(table, "abb-key", "abb"))
		return TST_FAILED;

	tst_msg("Freeing table");

	gp_htable_free(table);

	return TST_PASSED;
}

static int double_collision_test(void)
{
	gp_htable *table = gp_htable_new(0, 0);

	if (!table) {
		tst_msg("Malloc failed :-(");
		return TST_FAILED;
	}

	tst_msg("Inserting 'aa-key' size=%zu", table->size);
	gp_htable_put2(table, stupid_hash, "aa", "aa-key");
	tst_msg("Inserting 'ba-key' size=%zu", table->size);
	gp_htable_put2(table, stupid_hash, "ba", "ba-key");
	tst_msg("Inserting 'ab-key' size=%zu", table->size);
	gp_htable_put2(table, stupid_hash, "ab", "ab-key");
	tst_msg("Removing 'aa-key'");
	gp_htable_rem2(table, stupid_hash, gp_htable_strcmp, "aa-key");

	if (lookup_success(table, "ab-key", "ab"))
		return TST_FAILED;

	gp_htable_free(table);

	return TST_PASSED;
}

static int check_size(gp_htable *table, size_t exp_size)
{
	if (table->size != exp_size) {
		tst_msg("Wrong table size %zu expected %zu records %zu",
		        table->size, exp_size, table->used);
		return TST_FAILED;
	}

	tst_msg("Table size %zu records %zu", table->size, table->used);

	return TST_PASSED;
}

static int size_test(void)
{
	gp_htable *table = gp_htable_new(0, 0);

	if (check_size(table, 3))
		return TST_FAILED;

	gp_htable_put(table, NULL, "aa");

	if (check_size(table, 3))
		return TST_FAILED;

	gp_htable_put(table, NULL, "ab");
	gp_htable_put(table, NULL, "ac");

	if (check_size(table, 7))
		return TST_FAILED;

	gp_htable_put(table, NULL, "ad");
	gp_htable_put(table, NULL, "ae");
	gp_htable_put(table, NULL, "af");
	gp_htable_put(table, NULL, "ag");
	gp_htable_put(table, NULL, "ah");
	gp_htable_put(table, NULL, "ai");

	if (check_size(table, 19))
		return TST_FAILED;

	gp_htable_rem(table, "aa");
	gp_htable_rem(table, "ab");
	gp_htable_rem(table, "ac");
	gp_htable_rem(table, "ad");
	gp_htable_rem(table, "ae");
	gp_htable_rem(table, "af");
	gp_htable_rem(table, "ag");

	if (check_size(table, 19))
		return TST_FAILED;

	gp_htable_rem(table, "ai");

	if (check_size(table, 3))
		return TST_FAILED;

	tst_msg("Freeing table");

	gp_htable_free(table);

	return TST_PASSED;
}

static int foreach_test(void)
{
	gp_htable *table = gp_htable_new(0, 0);
	size_t elems = 0;

	gp_htable_put(table, NULL, "ab");
	gp_htable_put(table, NULL, "ac");

	GP_HTABLE_FOREACH(table, rec) {
		tst_msg("Record %s", (char*)rec->key);
		elems++;
	}

	if (elems != 2) {
		tst_msg("Wrong number of records!");
		return TST_FAILED;
	}

	gp_htable_free(table);

	return TST_PASSED;
}

/*
 * Keys are indexes + 1 into an array of strings passed as priv, so the hash
 * and compare functions cannot do anything without it.
 */
static size_t idx_hash(const void *key, size_t size, void *priv)
{
	const char **strs = priv;

	return gp_htable_strhash(strs[(uintptr_t)key - 1], size, NULL);
}

static int idx_cmp(const void *key1, const void *key2, void *priv)
{
	const char **strs = priv;

	return !strcmp(strs[(uintptr_t)key1 - 1], strs[(uintptr_t)key2 - 1]);
}

static int priv_test(void)
{
	gp_htable *table = gp_htable_new(0, 0);
	const char *strs[] = {"aa", "ab", "ac", "ad", "ae", "af", "ag", "ah",
	                      "ai", "aj", "ak", "al", "am", "an", "ao", "ap",
	                      NULL};
	uintptr_t i, n = 16;
	size_t size;

	if (!table) {
		tst_msg("Malloc failed :-(");
		return TST_FAILED;
	}

	/* inserts rehash the table several times */
	for (i = 0; i < n; i++) {
		void *key = (void *)(i + 1);

		gp_htable_put2_priv(table, idx_hash, key, key, strs);
	}

	size = table->size;

	/* removals repack the probe sequences and finally shrink the table */
	for (i = 0; i < n; i++) {
		void *key = (void *)(i + 1);

		if (i % 8 == 7)
			continue;

		if (gp_htable_rem2_priv(table, idx_hash, idx_cmp, key, strs) != key) {
			tst_msg("Wrong value removing '%s'", strs[i]);
			return TST_FAILED;
		}
	}

	/* look up by a key that is not in the table but names an equal string */
	for (i = 0; i < n; i++) {
		uintptr_t val;

		strs[n] = strs[i];
		val = (uintptr_t)gp_htable_get2_priv(table, idx_hash, idx_cmp,
		                                     (void *)(n + 1), strs);

		if (val != (i % 8 == 7 ? i + 1 : 0)) {
			tst_msg("Wrong value %zu for '%s'", (size_t)val, strs[i]);
			return TST_FAILED;
		}
	}

	if (table->size >= size) {
		tst_msg("Table did not shrink, size %zu", table->size);
		return TST_FAILED;
	}

	tst_msg("Table size %zu -> %zu records %zu",
	        size, table->size, table->used);

	gp_htable_free(table);

	return TST_PASSED;
}

const struct tst_suite tst_suite = {
	.suite_name = "htable testsuite",
	.tests = {
		{.name = "htable test",
		 .tst_fn = htable_test,
		 .flags = TST_CHECK_MALLOC},

		{.name = "htable collision test",
		 .tst_fn = collision_test,
		 .flags = TST_CHECK_MALLOC},

		{.name = "htable double collision test",
		 .tst_fn = double_collision_test,
		 .flags = TST_CHECK_MALLOC},

		{.name = "htable size test",
		 .tst_fn = size_test,
		 .flags = TST_CHECK_MALLOC},

		{.name = "htable foreach test",
		 .tst_fn = foreach_test},

		{.name = "htable priv test",
		 .tst_fn = priv_test,
		 .flags = TST_CHECK_MALLOC},

		{}
	}
};
