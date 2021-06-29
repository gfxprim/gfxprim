// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2009-2021 Cyril Hrubis <metan@ucw.cz>
 */

#include <inttypes.h>
#include <string.h>

#include <core/gp_debug.h>
#include <core/gp_common.h>
#include <input/gp_timer.h>
#include <utils/gp_heap.h>

static void dump_level(gp_heap_head *heap, unsigned int level, unsigned int cur)
{
	gp_timer *timer = GP_HEAP_ENTRY(heap, struct gp_timer, heap);

	if (level == cur) {
		if (heap)
			printf("[%9s %8"PRIu64"] ", timer->id, timer->expires);
		else
			printf("                    ");
		return;
	}

	dump_level(heap ? heap->left : NULL, level, cur+1);
	dump_level(heap ? heap->right : NULL , level, cur+1);
}

void gp_timer_queue_dump(gp_timer *heap)
{
	unsigned int i, j = 0;

	if (!heap)
		return;

	for (i = 1; heap->heap.children + 1 >= i; i = i*2) {
		dump_level(&heap->heap, j++, 0);
		printf("\n");
	}
}

static int timer_cmp(gp_heap_head *h1, gp_heap_head *h2)
{
	gp_timer *t1 = GP_HEAP_ENTRY(h1, struct gp_timer, heap);
	gp_timer *t2 = GP_HEAP_ENTRY(h2, struct gp_timer, heap);

	return t1->expires > t2->expires;
}

void gp_timer_queue_insert(gp_timer **queue, uint64_t now, gp_timer *timer)
{
	uint32_t after = timer->period ? timer->period : timer->expires;
	uint64_t expires = now + after;

	GP_DEBUG(3, "Inserting timer %s (now is %"PRIu64") expires after %"
	         PRIu32" at %"PRIu64, timer->id, now, after, expires);

	timer->expires = expires;

	gp_heap_head *head = gp_heap_ins(&(*queue)->heap, &timer->heap, timer_cmp);

	*queue = GP_HEAP_ENTRY(head, struct gp_timer, heap);
}

void gp_timer_queue_remove(gp_timer **queue, gp_timer *timer)
{
	GP_DEBUG(3, "Removing timer %s from queue", timer->id);

	if (!*queue) {
		GP_WARN("Attempt to remove timer %s from empty queue",
		        timer->id);
	}

	gp_heap_head *head = gp_heap_rem(&(*queue)->heap, &timer->heap, timer_cmp);

	*queue = GP_HEAP_ENTRY(head, struct gp_timer, heap);
}

static gp_timer *process_top(gp_timer *queue, uint64_t now)
{
	gp_timer *timer = queue;
	uint32_t ret, period;

	GP_DEBUG(3, "Timer %s expired at %"PRIu64" now is %"PRIu64,
	         timer->id, timer->expires, now);

	gp_heap_head *head = gp_heap_pop(&queue->heap, timer_cmp);

	period = timer->period;
	ret = timer->callback(timer);

	if (period)
		ret = period;

	if (ret) {
		timer->expires = now + ret;
		GP_DEBUG(3, "Rescheduling %stimer %s (now is %"PRIu64") after %"
		         PRIu32" expires at %"PRIu64,
		         period ? "periodic " : "",
		         timer->id, now, ret, timer->expires);
		head = gp_heap_ins(head, &timer->heap, timer_cmp);
	}

	return GP_HEAP_ENTRY(head, struct gp_timer, heap);
}

int gp_timer_queue_process(gp_timer **heap, uint64_t now)
{
	int ret = 0;

	for (;;) {
		if (!*heap)
			return ret;

		if ((*heap)->expires <= now) {
			*heap = process_top(*heap, now);
			ret++;
		} else {
			return ret;
		}
	}
}
