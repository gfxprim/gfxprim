//SPDX-License-Identifier: GPL-2.1-or-later

/*

    Copyright (C) 2022 Cyril Hrubis <metan@ucw.cz>

 */

/**
 * @file gp_trie.h
 * @brief An unicode trie.
 *
 * Unicode trie with AVL trees for nodes.
 */

#ifndef UTILS_GP_TRIE_H
#define UTILS_GP_TRIE_H

#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <core/gp_common.h>
#include <core/gp_compiler.h>
#include <utils/gp_avl_tree.h>
#include <utils/gp_utf.h>

/** @brief Trie node stores pointers. */
typedef struct gp_trie_node {
	void *payload;
	struct gp_trie_node *parent;

	/* A node key */
	uint32_t key;

	/* embedded pointers */
	gp_avl_node avl_next;

	/* children */
	gp_avl_node *nodes;
} gp_trie_node;

static inline void *gp_trie_payload(gp_trie_node *self)
{
	return self->payload;
}

static inline gp_trie_node *gp_trie_node_new(void)
{
	struct gp_trie_node *node = malloc(sizeof(gp_trie_node));

	if (!node)
		return NULL;

	memset(node, 0, sizeof(*node));

	return node;
}

static inline void gp_trie_free(gp_trie_node *root, void (*payload_free)(void *payload))
{
	gp_avl_node *avl_root, *min;
	gp_trie_node *node;

	if (!root)
		return;

	avl_root = root->nodes;

	if (avl_root) {
		do {
			avl_root = gp_avl_tree_del_min(avl_root, &min);
			node = GP_CONTAINER_OF(min, gp_trie_node, avl_next);
			gp_trie_free(node, payload_free);
		} while (avl_root);
	}

	if (payload_free)
		payload_free(root->payload);

	free(root);
}

static inline int gp_trie_cmp(gp_avl_node *a, gp_avl_node *b)
{
	gp_trie_node *ta = GP_CONTAINER_OF(a, gp_trie_node, avl_next);
	gp_trie_node *tb = GP_CONTAINER_OF(b, gp_trie_node, avl_next);

	return ta->key - tb->key;
}

static inline int gp_trie_key_cmp(gp_avl_node *a, const void *key)
{
	gp_trie_node *ta = GP_CONTAINER_OF(a, gp_trie_node, avl_next);
	uint32_t kval = *(const uint32_t*)key;

	return ta->key - kval;
}

GP_WUR static inline gp_trie_node *gp_trie_ins(gp_trie_node *root, const char *str_key, void *payload)
{
	uint32_t key = gp_utf8_next(&str_key);
	gp_trie_node *node = NULL;
	gp_avl_node *avl_node;

	if (!root) {
		root = gp_trie_node_new();
		if (!root)
			return NULL;
	}

	if (!key) {
		root->payload = payload;
		return root;
	}

	avl_node = gp_avl_tree_lookup(root->nodes, &key, gp_trie_key_cmp);
	if (avl_node)
		node = GP_CONTAINER_OF(avl_node, gp_trie_node, avl_next);

	node = gp_trie_ins(node, str_key, payload);

	if (!node)
		return root;

	if (!avl_node) {
		node->key = key;
		root->nodes = gp_avl_tree_ins(root->nodes, &node->avl_next, gp_trie_cmp);
	}

	node->parent = root;
	return root;
}

static inline gp_trie_node *gp_trie_lookup(gp_trie_node *root, const char *str_key)
{
	uint32_t key = gp_utf8_next(&str_key);
	gp_avl_node *avl_node;

	if (!key)
		return root;

	if (!root)
		return NULL;

	avl_node = gp_avl_tree_lookup(root->nodes, &key, gp_trie_key_cmp);
	if (!avl_node)
		return NULL;

	return gp_trie_lookup(GP_CONTAINER_OF(avl_node, gp_trie_node, avl_next), str_key);
}

GP_WUR static inline gp_trie_node *gp_trie_del_(gp_trie_node *root, const char *str_key, void **payload)
{
	uint32_t key = gp_utf8_next(&str_key);
	gp_trie_node *node = NULL;
	gp_avl_node *avl_node;

	if (!root)
		return NULL;

	if (!key) {
		if (payload)
			*payload = root->payload;

		if (root->nodes) {
			root->payload = NULL;
			return NULL;
		}

		return root;
	}

	avl_node = gp_avl_tree_lookup(root->nodes, &key, gp_trie_key_cmp);
	if (avl_node)
		node = GP_CONTAINER_OF(avl_node, gp_trie_node, avl_next);

	node = gp_trie_del_(node, str_key, payload);
	if (node) {
		root->nodes = gp_avl_tree_del(root->nodes, &key, NULL, gp_trie_key_cmp);
		free(node);
	}

	if (root->payload)
		return NULL;

	if (root->nodes)
		return NULL;

	return root;
}

GP_WUR static inline gp_trie_node *gp_trie_del(gp_trie_node *root, const char *str_key, void **payload)
{
	gp_trie_node *node = gp_trie_del_(root, str_key, payload);

	if (node) {
		free(root);
		return NULL;
	}

	return root;
}

#endif /* UTILS_GP_TRIE_H */
