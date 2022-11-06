//SPDX-License-Identifier: LGPL-2.0-or-later

/*

   Copyright (c) 2007 - 2022 Cyril Hrubis <metan@ucw.cz>

 */

#ifndef UTILS_GP_AVL_TREE_H
#define UTILS_GP_AVL_TREE_H

typedef struct gp_avl_node {
	//void *up;
	void *left;
	void *right;
	unsigned long depth;
} gp_avl_node;

static inline unsigned int gp_avl_tree_depth_(gp_avl_node *root)
{
	if (!root)
		return 0;

	return root->depth;
}

static inline int gp_avl_tree_depth_diff_(gp_avl_node *root)
{
	return gp_avl_tree_depth_(root->left) - gp_avl_tree_depth_(root->right);
}

static inline int gp_avl_tree_left_deeper_(gp_avl_node *root)
{
	return gp_avl_tree_depth_diff_(root) >= 0;
}

static inline int gp_avl_tree_right_deeper_(gp_avl_node *root)
{
	return gp_avl_tree_depth_diff_(root) <= 0;
}

static inline void gp_avl_tree_update_subtree_depth_(gp_avl_node *root)
{
	unsigned long left  = gp_avl_tree_depth_(root->left);
	unsigned long right = gp_avl_tree_depth_(root->right);

	if (left > right)
		right = left;

	root->depth = right + 1;
}

__attribute__((warn_unused_result))
static inline gp_avl_node *gp_avl_tree_rotate_left_(gp_avl_node *root)
{
	gp_avl_node *pivot = root->left;

	root->left = pivot->right;
	pivot->right = root;

	gp_avl_tree_update_subtree_depth_(root);
	gp_avl_tree_update_subtree_depth_(pivot);

	return pivot;
}

__attribute__((warn_unused_result))
static inline gp_avl_node *gp_avl_tree_rotate_right_(gp_avl_node *root)
{
	gp_avl_node *pivot = root->right;

	root->right = pivot->left;
	pivot->left = root;

	gp_avl_tree_update_subtree_depth_(root);
	gp_avl_tree_update_subtree_depth_(pivot);

	return pivot;
}

__attribute__((warn_unused_result))
static inline gp_avl_node *gp_avl_tree_rotate_left_right_(gp_avl_node *root)
{
	root->left = gp_avl_tree_rotate_right_(root->left);

	return gp_avl_tree_rotate_left_(root);
}

__attribute__((warn_unused_result))
static inline gp_avl_node *gp_avl_tree_rotate_right_left_(gp_avl_node *root)
{
	root->right = gp_avl_tree_rotate_left_(root->right);

	return gp_avl_tree_rotate_right_(root);
}

__attribute__((warn_unused_result))
static inline gp_avl_node *gp_avl_tree_balance_(gp_avl_node *root)
{
	switch (gp_avl_tree_depth_diff_(root)) {
	case 2:
		if (gp_avl_tree_left_deeper_(root->left))
			root = gp_avl_tree_rotate_left_(root);
		else
			root = gp_avl_tree_rotate_left_right_(root);
	break;
	case -2:
		if (gp_avl_tree_right_deeper_(root->right))
			root = gp_avl_tree_rotate_right_(root);
		else
			root = gp_avl_tree_rotate_right_left_(root);
	break;
	default:
		gp_avl_tree_update_subtree_depth_(root);
	}

	return root;
}

__attribute__((warn_unused_result))
static inline gp_avl_node *gp_avl_tree_ins_(gp_avl_node *root, gp_avl_node *node, int (*cmp)(gp_avl_node *a, gp_avl_node *b))
{
	if (!root)
		return node;

	if (cmp(root, node) < 0)
		root->left = gp_avl_tree_ins_(root->left, node, cmp);
	else
		root->right = gp_avl_tree_ins_(root->right, node, cmp);

	return gp_avl_tree_balance_(root);
}

__attribute__((warn_unused_result))
static inline gp_avl_node *gp_avl_tree_ins(gp_avl_node *root, gp_avl_node *node, int (*cmp)(gp_avl_node *a, gp_avl_node *b))
{
	node->depth = 1;
	node->left  = NULL;
	node->right = NULL;

	return gp_avl_tree_ins_(root, node, cmp);
}

__attribute__((warn_unused_result))
static inline gp_avl_node *gp_avl_tree_del_min(gp_avl_node *root, gp_avl_node **min)
{
	if (!root->right) {
		gp_avl_node *left = root->left;

		*min = root;

		root->left = NULL;

		return left;
	}

	root->right = gp_avl_tree_del_min(root->right, min);

	if (root->right)
		gp_avl_tree_update_subtree_depth_(root->right);

	return gp_avl_tree_balance_(root);
}

__attribute__((warn_unused_result))
static inline gp_avl_node *gp_avl_tree_del_max(gp_avl_node *root, gp_avl_node **max)
{
	if (!root->left) {
		gp_avl_node *right = root->right;

		*max = root;

		root->right = NULL;

		return right;
	}

	root->left = gp_avl_tree_del_max(root->left, max);

	if (root->left)
		gp_avl_tree_update_subtree_depth_(root->left);

	return gp_avl_tree_balance_(root);
}

__attribute__((warn_unused_result))
static inline gp_avl_node *gp_avl_tree_del(gp_avl_node *root, const void *key, gp_avl_node **del,
                                           int (*cmp)(gp_avl_node *a, const void *key))
{
	int res;

	if (!root)
		return NULL;

	res = cmp(root, key);

	if (res < 0)
		root->left = gp_avl_tree_del(root->left, key, del, cmp);

	if (res > 0)
		root->right = gp_avl_tree_del(root->right, key, del, cmp);

	if (res == 0) {
		gp_avl_node *left = root->left;
		gp_avl_node *right = root->right;
		gp_avl_node *min;

		if (del)
			*del = root;

		if (!root->left)
			return root->right;

		if (!root->right)
			return root->left;

		left = gp_avl_tree_del_min(left, &min);

		min->left = left;
		min->right = right;

		return gp_avl_tree_balance_(root);
	}

	return gp_avl_tree_balance_(root);
}

static inline unsigned long gp_avl_tree_depth(gp_avl_node *root)
{
	if (!root)
		return 0;

	return root->depth;
}

static inline gp_avl_node *gp_avl_tree_lookup(gp_avl_node *root, const void *key,
                                              int (*cmp)(gp_avl_node *a, const void *key))
{
	int ret;

	if (!root)
		return NULL;

	ret = cmp(root, key);
	if (ret == 0)
		return root;

	if (ret < 0)
		return gp_avl_tree_lookup(root->left, key, cmp);
	else
		return gp_avl_tree_lookup(root->right, key, cmp);
}

#endif /* UTILS_GP_AVL_TREE_H */
