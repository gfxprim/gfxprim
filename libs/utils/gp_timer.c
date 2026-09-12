// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2009-2026 Cyril Hrubis <metan@ucw.cz>
 */

#include <inttypes.h>
#include <string.h>

#include <core/gp_debug.h>
#include <core/gp_common.h>
#include <utils/gp_timer.h>
#include <utils/gp_avl_tree.h>
#include <utils/gp_list.h>

#define TIMER_NODE(node) GP_CONTAINER_OF(node, gp_timer, avl)
#define TIMER_ENTRY(entry) GP_LIST_ENTRY(entry, gp_timer, lh)

static int expires_cmp(uint64_t e1, uint64_t e2)
{
	if (e1 == e2)
		return 0;

	return e1 < e2 ? -1 : 1;
}

static int bucket_cmp(gp_avl_node *n1, gp_avl_node *n2)
{
	return expires_cmp(TIMER_NODE(n1)->expires, TIMER_NODE(n2)->expires);
}

static int bucket_cmp_key(gp_avl_node *node, const void *key)
{
	return expires_cmp(TIMER_NODE(node)->expires, *(const uint64_t *)key);
}

static gp_timer *bucket_lookup(gp_timer_queue *self, uint64_t expires)
{
	gp_avl_node *node = gp_avl_tree_lookup(self->root, &expires, bucket_cmp_key);

	if (!node)
		return NULL;

	return TIMER_NODE(node);
}

static void dump_buckets(gp_avl_node *node)
{
	gp_dlist_head *i;
	gp_timer *head;

	if (!node)
		return;

	dump_buckets(node->right);

	head = TIMER_NODE(node);

	printf("%9"PRIu64":", head->expires);

	GP_CLIST_FOREACH(&head->lh, i)
		printf(" %s", TIMER_ENTRY(i)->id);

	printf("\n");

	dump_buckets(node->left);
}

void gp_timer_queue_dump(const gp_timer_queue *self)
{
	dump_buckets(self->root);
}

gp_timer *gp_timer_queue_first(const gp_timer_queue *self)
{
	const gp_avl_node *node = self->root;

	if (!node)
		return NULL;

	node = gp_avl_tree_min(node);

	return GP_CONTAINER_OF(node, gp_timer, avl);
}

static void queue_ins(gp_timer_queue *self, gp_timer *timer)
{
	gp_timer *head = bucket_lookup(self, timer->expires);

	self->size++;

	if (head) {
		timer->bucket_head = 0;
		gp_clist_push_tail(&head->lh, &timer->lh);
		return;
	}

	timer->bucket_head = 1;
	gp_clist_init(&timer->lh);
	self->root = gp_avl_tree_ins(self->root, &timer->avl, bucket_cmp);
}

static void queue_rem(gp_timer_queue *self, gp_timer *timer)
{
	gp_timer *next = TIMER_ENTRY(timer->lh.next);

	self->size--;

	gp_clist_rem(&timer->lh);

	if (!timer->bucket_head)
		return;

	timer->bucket_head = 0;

	self->root = gp_avl_tree_del(self->root, &timer->expires, NULL, bucket_cmp_key);

	/* It was the only timer in the bucket, the bucket goes with it. */
	if (next == timer)
		return;

	next->bucket_head = 1;

	self->root = gp_avl_tree_ins(self->root, &next->avl, bucket_cmp);
}

static void stop_timer(gp_timer *self)
{
	int free_on_stop = self->free_on_stop;

	self->running = 0;
	self->expires = 0;

	if (self->stopped)
		self->stopped(self);

	if (free_on_stop)
		gp_timer_free(self);
}

void gp_timer_queue_ins(gp_timer_queue *self, uint64_t now, gp_timer *timer)
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

	if (timer->reschedule) {
		GP_DEBUG(3, "Timer %s re-inserted while waiting for reschedule", timer->id);
		return;
	}

	queue_ins(self, timer);
}

void gp_timer_queue_rem(gp_timer_queue *self, gp_timer *timer)
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

	if (timer->reschedule) {
		GP_DEBUG(3, "Timer %s removed while waiting to be rescheduled",
		         timer->id);
		timer->expires = GP_TIMER_STOP;
		return;
	}

	if (!self->root) {
		GP_WARN("Attempt to remove timer %s from empty queue",
		        timer->id);
		return;
	}

	queue_rem(self, timer);
	stop_timer(timer);
}

static void process_top(gp_timer_queue *self, gp_timer *timer, gp_dlist *reschedule, uint64_t now)
{
	uint32_t ret;

	GP_DEBUG(3, "Timer %s expired at %"PRIu64" now is %"PRIu64,
	         timer->id, timer->expires, now);

	queue_rem(self, timer);

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
		stop_timer(timer);
		return;
	}

	timer->expires = ret + now;

	GP_DEBUG(3, "Rescheduling timer '%s' after %"PRIu32" expires at %"PRIu64,
	         timer->id, ret, timer->expires);

	/* Timer that is will not expire second time can be inserted right back. */
	if (timer->expires > now) {
		queue_ins(self, timer);
		return;
        }

	timer->reschedule = 1;
	gp_dlist_push_tail(reschedule, &timer->lh);
}

int gp_timer_queue_process(gp_timer_queue *self, uint64_t now)
{
	gp_dlist reschedule = {};
	gp_dlist_head *entry;
	int ret = 0;

	for (;;) {
		gp_timer *first = gp_timer_queue_first(self);

		if (!first || first->expires > now)
			break;

		process_top(self, first, &reschedule, now);
		ret++;
	}

	while ((entry = gp_dlist_pop_head(&reschedule))) {
		gp_timer *timer = TIMER_ENTRY(entry);

		timer->reschedule = 0;

		if (timer->expires == GP_TIMER_STOP)
			stop_timer(timer);
		else
			queue_ins(self, timer);
	}

	return ret;
}

void gp_timer_free(gp_timer *self)
{
	if (!self)
		return;

	if (self->running) {
		self->free_on_stop = 1;
		return;
	}

	GP_DEBUG(1, "Freeing timer '%s'", self->id);

	free(self);
}

gp_timer *gp_timer_alloc(uint32_t expires_ms, uint32_t period_ms, const char *id,
                         uint32_t (*callback)(gp_timer *), void *priv)
{
	size_t size = sizeof(gp_timer) + strlen(id) + 1;

	GP_DEBUG(1, "Allocating timer %s", id);

	gp_timer *ret = malloc(size);
	if (!ret) {
		GP_WARN("Malloc failed :(");
		return NULL;
	}

	memset(ret, 0, size);

	strcpy(ret->data, id);

	ret->id = ret->data;
	ret->expires = expires_ms;
	ret->period = period_ms;
	ret->callback = callback;
	ret->priv = priv;

	return ret;
}
