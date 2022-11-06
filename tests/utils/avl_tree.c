// SPDX-License-Identifier: GPL-2.1-or-later
/*

  Copyright (C) 2022 Cyril Hrubis <metan@ucw.cz>

 */

#include <core/gp_common.h>
#include <utils/gp_avl_tree.h>

#include "tst_test.h"

struct avl_tree_entry {
	unsigned int key;
	gp_avl_node node;
};

static int cmp_nodes(gp_avl_node *a, gp_avl_node *b)
{
	struct avl_tree_entry *ea = GP_CONTAINER_OF(a, struct avl_tree_entry, node);
	struct avl_tree_entry *eb = GP_CONTAINER_OF(b, struct avl_tree_entry, node);

	return ea->key - eb->key;
}

static int cmp_key(gp_avl_node *a, const void *key)
{
	struct avl_tree_entry *ea = GP_CONTAINER_OF(a, struct avl_tree_entry, node);
	unsigned int kval = *(const int *)key;

	return ea->key - kval;
}

static struct avl_tree_entry entries[] = {
	{.key = 5},
	{.key = 4},
	{.key = 6},
	{.key = 7},
	{.key = 10},
	{.key = 1},
	{.key = 3},
	{.key = 14},
	{.key = 2},
	{.key = 8},
	{.key = 9},
	{.key = 11},
	{.key = 13},
	{.key = 12},
	{.key = 15},
};

static int avl_tree_lookup(void)
{
	gp_avl_node *root = NULL, *ret;
	struct avl_tree_entry *entry;
	int fail = 0;
	unsigned int i;

	for (i = 0; i < (int)GP_ARRAY_SIZE(entries); i++)
		root = gp_avl_tree_ins(root, &(entries[i].node), cmp_nodes);

	if (root->depth != 5) {
		tst_msg("Wrong tree depth %lu\n", root->depth);
		return TST_FAILED;
	}

	for (i = 1; i <= GP_ARRAY_SIZE(entries); i++) {
		ret = gp_avl_tree_lookup(root, &i, cmp_key);

		if (!ret) {
			tst_msg("Lookup %u failed", i);
			fail++;
			continue;
		}

		entry = GP_CONTAINER_OF(ret, struct avl_tree_entry, node);
		if (entry->key != i) {
			tst_msg("Wrong lookup result");
			fail++;
		}
	}

	i = 666;

	ret = gp_avl_tree_lookup(root, &i, cmp_key);
	if (ret) {
		tst_msg("Got node for nonexistent key");
		fail++;
	}

	if (fail)
		return TST_FAILED;

	return TST_SUCCESS;
}

static int avl_tree_del_min(void)
{
	gp_avl_node *root = NULL, *ret;
	struct avl_tree_entry *entry;
	int fail = 0;
	unsigned int i;

	for (i = 0; i < (int)GP_ARRAY_SIZE(entries); i++)
		root = gp_avl_tree_ins(root, &(entries[i].node), cmp_nodes);

	if (root->depth != 5) {
		tst_msg("Wrong tree depth %lu\n", root->depth);
		return TST_FAILED;
	}

	for (i = 1; i <= GP_ARRAY_SIZE(entries); i++) {
		root = gp_avl_tree_del_min(root, &ret);

		if (!ret) {
			tst_msg("Tree empty");
			fail++;
			continue;
		}

		entry = GP_CONTAINER_OF(ret, struct avl_tree_entry, node);

		if (entry->key != i) {
			tst_msg("wrong min %u", entry->key);
			fail++;
		}
	}

	if (root) {
		tst_msg("Tree is not empty");
		fail++;
	}

	if (fail)
		return TST_FAILED;

	return TST_SUCCESS;
}

static int avl_tree_del_max(void)
{
	gp_avl_node *root = NULL, *ret;
	struct avl_tree_entry *entry;
	int fail = 0;
	unsigned int i;

	for (i = 0; i < (int)GP_ARRAY_SIZE(entries); i++)
		root = gp_avl_tree_ins(root, &(entries[i].node), cmp_nodes);

	if (root->depth != 5) {
		tst_msg("Wrong tree depth %lu\n", root->depth);
		return TST_FAILED;
	}

	for (i = GP_ARRAY_SIZE(entries); i > 0; i--) {
		root = gp_avl_tree_del_max(root, &ret);

		if (!ret) {
			tst_msg("Tree empty");
			fail++;
			continue;
		}

		entry = GP_CONTAINER_OF(ret, struct avl_tree_entry, node);

		if (entry->key != i) {
			tst_msg("wrong min %u", entry->key);
			fail++;
		}
	}

	if (root) {
		tst_msg("Tree is not empty");
		fail++;
	}

	if (fail)
		return TST_FAILED;

	return TST_SUCCESS;
}

static gp_avl_node *del_entry(gp_avl_node *root, unsigned int key)
{
	gp_avl_node *ret;
	struct avl_tree_entry *entry;

	root = gp_avl_tree_del(root, &key, &ret, cmp_key);
	if (!ret) {
		tst_msg("Failed to remove key!");
		return NULL;
	}

	entry = GP_CONTAINER_OF(ret, struct avl_tree_entry, node);
	if (entry->key != key) {
		tst_msg("Wrong entry removed key %u", entry->key);
		return NULL;
	}

	if (!root)
		tst_msg("Empty tree on del");

	return root;
}

static int avl_tree_del(void)
{
	gp_avl_node *root = NULL, *ret;
	struct avl_tree_entry *entry;
	int fail = 0;
	unsigned int i;

	for (i = 0; i < (int)GP_ARRAY_SIZE(entries); i++)
		root = gp_avl_tree_ins(root, &(entries[i].node), cmp_nodes);

	root = del_entry(root, 6);
	if (!root)
		return TST_FAILED;

	root = del_entry(root, 9);
	if (!root)
		return TST_FAILED;

	for (i = GP_ARRAY_SIZE(entries); i > 0; i--) {

		if (i == 6 || i == 9)
			continue;

		root = gp_avl_tree_del_max(root, &ret);

		if (!ret) {
			tst_msg("Tree empty");
			fail++;
			continue;
		}

		entry = GP_CONTAINER_OF(ret, struct avl_tree_entry, node);

		if (entry->key != i) {
			tst_msg("wrong min %u", entry->key);
			fail++;
		}
	}

	if (root) {
		tst_msg("Tree is not empty");
		fail++;
	}

	if (fail)
		return TST_FAILED;

	return TST_SUCCESS;
}

const struct tst_suite tst_suite = {
	.suite_name = "avl tree testsuite",
	.tests = {
		{.name = "avl tree lookup",
		 .tst_fn = avl_tree_lookup,
		 .flags = TST_CHECK_MALLOC},

		{.name = "avl tree del min",
		 .tst_fn = avl_tree_del_min,
		 .flags = TST_CHECK_MALLOC},

		{.name = "avl tree del max",
		 .tst_fn = avl_tree_del_max,
		 .flags = TST_CHECK_MALLOC},

		{.name = "avl tree del",
		 .tst_fn = avl_tree_del,
		 .flags = TST_CHECK_MALLOC},

		{}
	}
};
