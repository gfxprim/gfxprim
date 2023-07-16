// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2009-2023 Cyril Hrubis <metan@ucw.cz>
 */

/*

   Generic min/max heap implementation.

 */

#ifndef UTILS_GP_HEAP_H
#define UTILS_GP_HEAP_H

#include <string.h>
#include <core/gp_debug.h>
#include <core/gp_common.h>
#include <utils/gp_types.h>

#define GP_HEAP_ENTRY(ptr, structure, member) \
	GP_CONTAINER_OF(ptr, structure, member)

static inline unsigned long gp_heap_size(gp_heap_head *heap)
{
	return heap ? heap->children + 1 : 0;
}

/*
 * If needed fixes up->left or up->right
 */
static inline void gp_heap_fix_up(gp_heap_head *up, gp_heap_head *old, gp_heap_head *rep)
{
	if (!up)
		return;

	if (old == up->left)
		up->left = rep;
	else if (old == up->right)
		up->right = rep;
	else
		GP_BUG("Invalid heap state");
}

/*
 * If needed fixes elem->left->up and elem->right->up
 */
static inline void gp_heap_fix_up_lr(gp_heap_head *elem, gp_heap_head *up)
{
	if (elem->left)
		elem->left->up = up;

	if (elem->right)
		elem->right->up = up;
}

/*
 * Swaps left child with its parent.
 */
static inline gp_heap_head *gp_heap_swap_left(gp_heap_head *heap)
{
	gp_heap_head *left = heap->left;

	gp_heap_fix_up(heap->up, heap, left);

	left->up = heap->up;
	heap->up = left;

	if (heap->right)
		heap->right->up = left;

	gp_heap_fix_up_lr(left, heap);

	heap->left = left->left;

	if (left)
		left->left = heap;

	GP_SWAP(heap->right, left->right);
	GP_SWAP(heap->children, left->children);

	return left;
}

/*
 * Swaps right child with its parent.
 */
static inline gp_heap_head *gp_heap_swap_right(gp_heap_head *heap)
{
	gp_heap_head *right = heap->right;
	gp_heap_fix_up(heap->up, heap, right);

	right->up = heap->up;
	heap->up = right;

	if (heap->left)
		heap->left->up = right;

	gp_heap_fix_up_lr(right, heap);

	heap->right = right->right;
	right->right = heap;

	GP_SWAP(heap->left, right->left);
	GP_SWAP(heap->children, right->children);

	return right;
}

/*
 * Returns true if subtree is well balanced i.e. we have 2^n - 2 children
 */
static int gp_heap_well_balanced(unsigned int children)
{
	return !((children + 2) & (children + 1));
}

/*
 * Inserts an element into a heap.
 */
__attribute__((warn_unused_result))
gp_heap_head *gp_heap_ins_(gp_heap_head *heap, gp_heap_head *parent, gp_heap_head *elem,
                           int (*cmp)(gp_heap_head *e1, gp_heap_head *e2))
{
	if (!heap) {
		memset(elem, 0, sizeof(*elem));
		elem->up = parent;
		return elem;
	}

	heap->children++;

	if (!heap->left || !gp_heap_well_balanced(heap->left->children) ||
	     (heap->right && heap->left->children == heap->right->children)) {

		heap->left = gp_heap_ins_(heap->left, heap, elem, cmp);

		if (cmp(heap, heap->left))
			return gp_heap_swap_left(heap);
	} else {

		heap->right = gp_heap_ins_(heap->right, heap, elem, cmp);

		if (cmp(heap, heap->right))
			return gp_heap_swap_right(heap);
	}

	return heap;
}

__attribute__((warn_unused_result))
gp_heap_head *gp_heap_ins(gp_heap_head *heap, gp_heap_head *elem,
                          int (*cmp)(gp_heap_head *e1, gp_heap_head *e2))
{
	return gp_heap_ins_(heap, NULL, elem, cmp);
}

/*
 * Removes last element on the last level.
 *
 * The head pointer only changes if we remove last element from heap.
 *
 * @heap A heap pointer.
 * @last A pointer to store the last element to.
 * @return A pointer to new head.
 */
static inline gp_heap_head *gp_heap_rem_last(gp_heap_head *heap,
	gp_heap_head **last)
{
	if (!heap->left) {
		*last = heap;
		return NULL;
	}

	if (!gp_heap_well_balanced(heap->left->children) ||
	    !heap->right || (heap->right->children < heap->left->children/2)) {
		heap->left = gp_heap_rem_last(heap->left, last);
	} else {
		heap->right = gp_heap_rem_last(heap->right, last);
	}

	heap->children--;

	return heap;
}

static inline gp_heap_head *gp_heap_bubble_down(gp_heap_head *heap,
	int (*cmp)(gp_heap_head *e1, gp_heap_head *e2))
{
	gp_heap_head *right = heap->right;
	gp_heap_head *left = heap->left;

	/* Make sure we choose smaller one */
	if (right && left && cmp(right, left))
		right = NULL;

	if (right && cmp(heap, right)) {
		gp_heap_swap_right(heap);
		right->right = gp_heap_bubble_down(heap, cmp);
		return right;
	}

	if (left && cmp(heap, left)) {
		gp_heap_swap_left(heap);
		left->left = gp_heap_bubble_down(heap, cmp);
		return left;
	}

	return heap;
}

/*
 * Removes top element from the heap.
 *
 * @heap Old heap pointer.
 * @return New heap pointer.
 */
__attribute__((warn_unused_result))
static inline gp_heap_head *gp_heap_pop(gp_heap_head *heap,
	int (*cmp)(gp_heap_head *e1, gp_heap_head *e2))
{
	gp_heap_head *last;

	if (!heap)
		return NULL;

	heap = gp_heap_rem_last(heap, &last);
	if (!heap)
		return NULL;

	last->left = heap->left;
	last->right = heap->right;
	last->children = heap->children;
	last->up = NULL;

	gp_heap_fix_up_lr(last, last);

	return gp_heap_bubble_down(last, cmp);
}

#include <assert.h>

/*
 * Moves replacement element up if needed.
 */
static inline gp_heap_head *gp_heap_bubble_up(gp_heap_head *heap,
	int (*cmp)(gp_heap_head *e1, gp_heap_head *e2))
{
	if (!heap || !heap->up)
		return heap;

	if (cmp(heap, heap->up))
		return heap;

	gp_heap_head *up = heap->up;

	if (up->left == heap)
		up = gp_heap_swap_left(up);
	else if (up->right == heap)
		up = gp_heap_swap_right(up);
	else
		GP_BUG("Invalid heap state");

	return gp_heap_bubble_up(up, cmp);
}

/**
 * Removes element from a heap.
 *
 * Does a buble down on a (sub) heap starting at the removed element and fixes
 * the links afterwards.
 */
__attribute__((warn_unused_result))
static inline gp_heap_head *gp_heap_rem(gp_heap_head *heap, gp_heap_head *elem,
	int (*cmp)(gp_heap_head *e1, gp_heap_head *e2))
{
	gp_heap_head *last;

	/* Heap is empty */
	if (!heap)
		return NULL;

	heap = gp_heap_rem_last(heap, &last);

	/* We found it already */
	if (elem == last)
		return heap;

	/* Replace removed element with last */
	last->left = elem->left;
	last->right = elem->right;
	last->children = elem->children;
	last->up = elem->up;

	gp_heap_fix_up(elem->up, elem, last);
	gp_heap_fix_up_lr(last, last);

	last = gp_heap_bubble_up(last, cmp);
	last = gp_heap_bubble_down(last, cmp);

	while (last->up)
		last = last->up;

	return last;
}

#endif /* UTILS_GP_HEAP_H */
