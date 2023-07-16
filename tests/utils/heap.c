// SPDX-License-Identifier: GPL-2.1-or-later
/*

  Copyright (C) 2023 Cyril Hrubis <metan@ucw.cz>

 */

#include <stdint.h>
#include <utils/gp_heap.h>

#include "tst_test.h"

struct heap_elem {
	gp_heap_head heap;
	int val;
};

static int cmp(gp_heap_head *a, gp_heap_head *b)
{
	struct heap_elem *ha = GP_HEAP_ENTRY(a, struct heap_elem, heap);
	struct heap_elem *hb = GP_HEAP_ENTRY(b, struct heap_elem, heap);

	return ha->val >= hb->val;
}

static int null_heap(void)
{
	gp_heap_head *heap = NULL;

	heap = gp_heap_rem(heap, heap, cmp);

	heap = gp_heap_pop(heap, cmp);

	tst_msg("Haven't crashed");
	return TST_PASSED;
}

static int heap_correct(gp_heap_head *heap, gp_heap_head *up)
{
	if (!heap)
		return 1;

	struct heap_elem *cur = GP_HEAP_ENTRY(heap, struct heap_elem, heap);

	if (heap->up != up) {
		tst_msg("Elem %i up does not match, have %p expected %p",
			cur->val, heap->up, up);
		return 0;
	}

	if (heap->left) {
		struct heap_elem *left = GP_HEAP_ENTRY(heap->left, struct heap_elem, heap);

		if (cur->val > left->val) {
			tst_msg("Elem %i > Elem->left %i", cur->val, left->val);
			return 0;
		}
	}

	if (heap->right) {
		struct heap_elem *right = GP_HEAP_ENTRY(heap->right, struct heap_elem, heap);

		if (cur->val > right->val) {
			tst_msg("Elem %i > Elem->right %i", cur->val, right->val);
			return 0;
		}
	}

	return heap_correct(heap->left, heap) && heap_correct(heap->right, heap);
}

static void dump_heap_level(gp_heap_head *heap, unsigned long level, unsigned long cur)
{
	struct heap_elem *elem = GP_HEAP_ENTRY(heap, struct heap_elem, heap);

	if (level == cur) {
		if (heap)
			printf("[%4i %p u %p l %p r %p] ", elem->val, heap, heap->up, heap->left, heap->right);
		else
			printf("                                              ");
		return;
	}

	dump_heap_level(heap ? heap->left : NULL, level, cur+1);
	dump_heap_level(heap ? heap->right : NULL , level, cur+1);
}

static void dump_heap(gp_heap_head *heap)
{
	unsigned long i, j = 0;

	if (!heap)
		return;

	for (i = 1; heap->children + 1 >= i; i = i*2) {
		dump_heap_level(heap, j++, 0);
		printf("\n");
	}
}

static int heap_insert(void)
{
	size_t i;
	struct heap_elem elems[] = {
		{.val = 1},
		{.val = 3},
		{.val = 2},
		{.val = 6},
		{.val = 4},
		{.val = 5},
	};

	gp_heap_head *heap = NULL;

	for (i = 0; i < GP_ARRAY_SIZE(elems); i++) {
		heap = gp_heap_ins(heap, &elems[i].heap, cmp);

		if (gp_heap_size(heap) != i + 1) {
			tst_msg("Wrong heap size %lu expected %zu\n", gp_heap_size(heap), i+1);
			return TST_FAILED;
		}

		if (!heap_correct(heap, NULL)) {
			dump_heap(heap);
			return TST_FAILED;
		}
	}

	return TST_PASSED;
}

#define BENCH_CNT 1000

static int heap_ins_pop_benchmark(void)
{
	size_t i;
	struct heap_elem *elems;
	gp_heap_head *heap = NULL;

	elems = malloc(sizeof(struct heap_elem) * BENCH_CNT);
	if (!elems)
		return TST_FAILED;

	for (i = 0; i < BENCH_CNT; i++) {
		elems[i].val = random() % BENCH_CNT;
		heap = gp_heap_ins(heap, &elems[i].heap, cmp);
		if (!heap_correct(heap, NULL)) {
			dump_heap(heap);
			return TST_FAILED;
		}
	}

	for (i = 0; i < BENCH_CNT; i++) {
		heap = gp_heap_pop(heap, cmp);
		if (!heap_correct(heap, NULL)) {
			dump_heap(heap);
			return TST_FAILED;
		}
	}

	return TST_PASSED;
}

static int heap_ins_rem_benchmark(void)
{
	size_t i;
	struct heap_elem *elems;
	gp_heap_head *heap = NULL;

	elems = malloc(sizeof(struct heap_elem) * BENCH_CNT);
	if (!elems)
		return TST_FAILED;

	for (i = 0; i < BENCH_CNT; i++) {
		elems[i].val = random() % BENCH_CNT;
		heap = gp_heap_ins(heap, &elems[i].heap, cmp);
		if (!heap_correct(heap, NULL)) {
			dump_heap(heap);
			return TST_FAILED;
		}
	}

	for (i = 0; i < BENCH_CNT; i++) {
		heap = gp_heap_rem(heap, &elems[i].heap, cmp);
		if (!heap_correct(heap, NULL))
			return TST_FAILED;
	}
	return TST_PASSED;
}

const struct tst_suite tst_suite = {
	.suite_name = "binary heap testsuite",
	.tests = {
		{.name = "null heap",
		 .tst_fn = null_heap},

		{.name = "heap insert",
		 .tst_fn = heap_insert},

		{.name = "ins pop benchmark",
		 .tst_fn = heap_ins_pop_benchmark,
		 .bench_iter = 1000},

		{.name = "ins rem benchmark",
		 .tst_fn = heap_ins_rem_benchmark,
		 .bench_iter = 1000},

		{}
	}
};
