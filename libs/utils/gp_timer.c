// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2009-2023 Cyril Hrubis <metan@ucw.cz>
 */

#include <inttypes.h>
#include <string.h>

#include <core/gp_debug.h>
#include <core/gp_common.h>
#include <utils/gp_timer.h>
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

	return t1->expires >= t2->expires;
}

void gp_timer_queue_ins(gp_timer **queue, uint64_t now, gp_timer *timer)
{
	uint32_t after = timer->expires;
	uint64_t expires = now + after;

	GP_DEBUG(3, "Inserting timer %s (now is %"PRIu64") expires after %"
	         PRIu32" at %"PRIu64" in_callback=%i",
		 timer->id, now, after, expires, timer->in_callback);

	if (timer->in_callback) {
		timer->expires = expires;
		timer->res_in_callback = 1;
		return;
	}

	if (timer->running) {
		GP_DEBUG(3, "Timer %s already running!", timer->id);
		return;
	}

	timer->expires = expires;
	timer->running = 1;

	gp_heap_head *head = gp_heap_ins(&(*queue)->heap, &timer->heap, timer_cmp);

	*queue = GP_HEAP_ENTRY(head, struct gp_timer, heap);
}

void gp_timer_queue_rem(gp_timer **queue, gp_timer *timer)
{
	GP_DEBUG(3, "Removing timer %s from queue in_callback=%i",
	         timer->id, timer->in_callback);

	if (!timer->running) {
		GP_DEBUG(3, "Timer %s is not running!", timer->id);
		return;
	}

	if (timer->in_callback) {
		timer->expires = GP_TIMER_STOP;
		timer->res_in_callback = 1;
		return;
	}

	timer->running = 0;
	timer->expires = 0;

	if (!*queue) {
		GP_WARN("Attempt to remove timer %s from empty queue",
		        timer->id);
		return;
	}

	gp_heap_head *head = gp_heap_rem(&(*queue)->heap, &timer->heap, timer_cmp);

	*queue = GP_HEAP_ENTRY(head, struct gp_timer, heap);
}

static gp_timer *process_top(gp_timer *queue, gp_timer **reschedulle, uint64_t now)
{
	gp_timer *timer = queue;
	uint32_t ret;

	GP_DEBUG(3, "Timer %s expired at %"PRIu64" now is %"PRIu64,
	         timer->id, timer->expires, now);

	gp_heap_head *head = gp_heap_pop(&queue->heap, timer_cmp);

	timer->in_callback = 1;

	ret = timer->callback(timer);
	if (timer->res_in_callback) {
		GP_DEBUG(3, "Timer '%s' changed from callback", timer->id);
		timer->res_in_callback = 0;
		ret = timer->expires;
		now = 0;
	}

	timer->in_callback = 0;

	if (ret == GP_TIMER_STOP) {
		timer->running = 0;
		timer->expires = 0;
	} else {
		timer->expires = ret + now;
		GP_DEBUG(3, "Rescheduling timer '%s' expires at %"PRIu64,
		         timer->id, timer->expires);
		timer->next = *reschedulle;
		*reschedulle = timer;
	}

	return GP_HEAP_ENTRY(head, struct gp_timer, heap);
}

int gp_timer_queue_process(gp_timer **queue, uint64_t now)
{
	int ret = 0;
	gp_timer *reschedulle = NULL, *tmp;

	for (;;) {
		if (!*queue)
			goto ret;

		if ((*queue)->expires <= now) {
			*queue = process_top(*queue, &reschedulle, now);
			ret++;
		} else {
			goto ret;
		}
	}
ret:
	gp_heap_head *heap = &(*queue)->heap;

	while (reschedulle) {
		tmp = reschedulle->next;
		heap = gp_heap_ins(heap, &reschedulle->heap, timer_cmp);
		reschedulle = tmp;
	}

	*queue = GP_HEAP_ENTRY(heap, struct gp_timer, heap);

	return ret;
}
