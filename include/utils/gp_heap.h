// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2009-2021 Cyril Hrubis <metan@ucw.cz>
 */

/*

   Generic min/max heap implementation.

 */

#ifndef UTILS_GP_HEAP_H
#define UTILS_GP_HEAP_H

#include <core/gp_common.h>
#include <utils/gp_types.h>

#define GP_HEAP_ENTRY(ptr, structure, member) \
	GP_CONTAINER_OF(ptr, structure, member)

static inline unsigned int gp_heap_size(gp_heap_head *heap)
{
	return heap ? heap->children + 1 : 0;
}

/*
 * Swaps left child with its parent.
 */
static inline gp_heap_head *gp_heap_swap_left(gp_heap_head *heap)
{
	gp_heap_head *left = heap->left;

	heap->left = left->left;
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
gp_heap_head *gp_heap_ins(gp_heap_head *heap, gp_heap_head *elem,
                          int (*cmp)(gp_heap_head *e1, gp_heap_head *e2))
{
	if (!heap) {
		memset(elem, 0, sizeof(*elem));
		return elem;
	}

	heap->children++;

	if (!heap->left || !gp_heap_well_balanced(heap->left->children) ||
	     (heap->right && heap->left->children == heap->right->children)) {

		heap->left = gp_heap_ins(heap->left, elem, cmp);

		if (cmp(heap, heap->left))
			return gp_heap_swap_left(heap);
	} else {

		heap->right = gp_heap_ins(heap->right, elem, cmp);

		if (cmp(heap, heap->right))
			return gp_heap_swap_right(heap);
	}

	return heap;
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
	    !heap->right || (heap->right->children < heap->left->children/2))
		heap->left = gp_heap_rem_last(heap->left, last);
	else
		heap->right = gp_heap_rem_last(heap->right, last);

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

	return gp_heap_bubble_down(last, cmp);
}

__attribute__((warn_unused_result))
static inline gp_heap_head *gp_heap_rem_(gp_heap_head *heap,
	gp_heap_head *elem, gp_heap_head *last,
	int (*cmp)(gp_heap_head *e1, gp_heap_head *e2), int *flag)
{
	if (!heap)
		return NULL;

	if (heap == elem) {
		*flag = 1;

		last->left = heap->left;
		last->right = heap->right;
		last->children = heap->children;

		return gp_heap_bubble_down(last, cmp);
	}

	heap->left = gp_heap_rem_(heap->left, elem, last, cmp, flag);
	heap->right = gp_heap_rem_(heap->right, elem, last, cmp, flag);

	return heap;
}

/*
 * Removes element from a heap.
 *
 * This operation (in contrast with insert and process) runs in O(n) time since
 * we travel the whole heap while searching for it.
 */
__attribute__((warn_unused_result))
static inline gp_heap_head *gp_heap_rem(gp_heap_head *heap, gp_heap_head *elem,
	int (*cmp)(gp_heap_head *e1, gp_heap_head *e2))
{
	gp_heap_head *last;
	int flag = 0;

	/* Heap is empty */
	if (!heap)
		return NULL;

	heap = gp_heap_rem_last(heap, &last);

	/* We found it already */
	if (elem == last)
		return heap;

	heap = gp_heap_rem_(heap, elem, last, cmp, &flag);

	/* Insert back the last element */
	if (!flag)
		return gp_heap_ins(heap, last, cmp);

	return heap;
}

#endif /* UTILS_GP_HEAP_H */
