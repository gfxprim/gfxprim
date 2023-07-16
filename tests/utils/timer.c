// SPDX-License-Identifier: GPL-2.1-or-later
/*
 * Copyright (C) 2009-2023 Cyril Hrubis <metan@ucw.cz>
 */

/*

  Test for Timers code.

 */

#include <string.h>
#include <stdlib.h>
#include <utils/gp_timer.h>

#include "tst_test.h"

static uint32_t callback_set_priv(gp_timer *self)
{
	self->priv = (void*)1;

	return GP_TIMER_STOP;
}

static int callback_is_called(void)
{
	GP_TIMER_DECLARE(timer, 10, 0, "Test", callback_set_priv, NULL);
	gp_timer *head = NULL;
	int fail = 0;

	gp_timer_queue_ins(&head, 10, &timer);

	/* Now call process before the timer expiration */
	if (gp_timer_queue_process(&head, 10)) {
		tst_msg("gp_timer_queue_process() reported non-zero");
		fail++;
	}

	if (timer.priv) {
		tst_msg("Callback was called");
		fail++;
	}

	/* Now call process after the expiration time */
	if (gp_timer_queue_process(&head, 30) != 1) {
		tst_msg("gp_timer_queue_process() reported wrong number");
		fail++;
	}

	if (!timer.priv) {
		tst_msg("Callback was not called");
		fail++;
	}

	if (gp_timer_queue_size(head)) {
		tst_msg("Timer queue is not empty after stopped timer");
		fail++;
	}

	if (fail)
		return TST_FAILED;

	return TST_PASSED;
}

#define MAX 1000000

static int monotonicity_failed = 0;
static uint64_t prev_expires;

static uint32_t callback_check_monotonicity(gp_timer *self)
{
	if (self->expires < prev_expires) {
		if (!monotonicity_failed)
			tst_msg("Wrong order of expirations detected");
		monotonicity_failed = 1;
	}

	prev_expires = self->expires;

	return GP_TIMER_STOP;
}

static int expirations_sorted(void)
{
	gp_timer *head = NULL;
	gp_timer *timers = malloc(sizeof(gp_timer) * MAX);
	int i;
	uint64_t expires;

	for (i = 0; i < MAX; i++) {
		timers[i].expires = random();
		timers[i].callback = callback_check_monotonicity;
		timers[i].priv = &expires;
		timers[i].id = "Test";
		gp_timer_queue_ins(&head, 0, &timers[i]);
	}

	prev_expires = head->expires;

	for (i = 0; i < MAX; i++)
		gp_timer_queue_process(&head, head ? head->expires : 0);

	if (monotonicity_failed)
		return TST_FAILED;

	if (gp_timer_queue_size(head)) {
		tst_msg("Some timers reschedulled?!");
		return TST_FAILED;
	}

	return TST_PASSED;
}

static int process_with_NULL_head(void)
{
	gp_timer *head = NULL;

	if (gp_timer_queue_process(&head, 1024)) {
		tst_msg("gp_timer_queue_process returned non-zero");
		return TST_FAILED;
	}

	return TST_PASSED;
}

static uint32_t callback_reschedulle(gp_timer *self)
{
	self->priv = (void*)1;

	return self->period;
}

/*
 * Test that periodic timers are rescheduled
 */
static int periodic_timers(void)
{
	GP_TIMER_DECLARE(timer1, 10, 10, "Test1", callback_reschedulle, NULL);
	GP_TIMER_DECLARE(timer2, 20, 20, "Test2", callback_reschedulle, NULL);
	gp_timer *head = NULL;
	int fail = 0;
	int ret;

	gp_timer_queue_ins(&head, 10, &timer1);
	gp_timer_queue_ins(&head, 10, &timer2);

	/* Make timer1 expire */
	ret = gp_timer_queue_process(&head, 20);
	if (ret != 1) {
		tst_msg("gp_timer_queue_process() reported %i expected 1", ret);
		fail++;
	}

	if (!timer1.priv) {
		tst_msg("Timer1 callback was not called");
		fail++;
	}

	/* check that there are two timers in the queue */
	if (head->heap.children != 1) {
		tst_msg("Queue head has wrong number of children %lu", head->heap.children);
		fail++;
	}

	timer1.priv = NULL;

	/* Make both timers expire */
	ret = gp_timer_queue_process(&head, 30);
	if (ret != 2) {
		tst_msg("gp_timer_queue_process() reported %i expected 2", ret);
		fail++;
	}

	if (!timer1.priv) {
		tst_msg("Timer1 callback was not called");
		fail++;
	}

	if (!timer2.priv) {
		tst_msg("Timer2 callback was not called");
		fail++;
	}

	/* check that there are two timers in the queue */
	if (head->heap.children != 1) {
		tst_msg("Queue head has wrong number of children %lu", head->heap.children);
		fail++;
	}

	if (fail)
		return TST_FAILED;

	return TST_PASSED;
}

/*
 * Special case where we remove a timer with the highest value, there was a bug
 * in the library heap that caused crash with this.
 */
static int rem_regression(void)
{
	GP_TIMER_DECLARE(timer1, 0, 10, "Test1", NULL, NULL);
	GP_TIMER_DECLARE(timer2, 0, 20, "Test2", NULL, NULL);
	gp_timer *head = NULL;

	gp_timer_queue_ins(&head, 10, &timer1);
	gp_timer_queue_ins(&head, 10, &timer2);

	gp_timer_queue_rem(&head, &timer2);

	if (head->heap.children) {
		tst_msg("Failed to remove timer");
		return TST_FAILED;
	}

	tst_msg("Timer removed correctly");

	return TST_PASSED;
}

static uint32_t callback_reschedulle_now(gp_timer *self)
{
	(void) self;
	return 0;
}

/*
 * Make sure that timer with zero expiration does not cause infinite loop in
 * gp_timer_queue_process()
 */
static int reschedulle_now(void)
{
	gp_timer *head = NULL;

	GP_TIMER_DECLARE(timer, 0, 0, "Test", callback_reschedulle_now, NULL);

	gp_timer_queue_ins(&head, 0, &timer);

	if (gp_timer_queue_process(&head, 0) != 1) {
		tst_msg("Wrong number of timers procesed");
		return TST_FAILED;
	}

	tst_msg("Call returned!");
	return TST_PASSED;
}

static int call_rem_from_cb(void *callback)
{
	gp_timer *head = NULL;

	GP_TIMER_DECLARE(timer, 0, 10, "Test", callback, &head);

	gp_timer_queue_ins(&head, 0, &timer);

	if (gp_timer_queue_process(&head, 10) != 1) {
		tst_msg("Wrong number of timers procesed");
		return TST_FAILED;
	}

	if (gp_timer_queue_size(head)) {
		tst_msg("Timer was not removed from a callback!");
		return TST_FAILED;
	}

	return TST_PASSED;
}

static int call_rem_ins_from_cb(void *callback)
{
	gp_timer *head = NULL;

	GP_TIMER_DECLARE(timer, 0, 0, "Test", callback, &head);

	gp_timer_queue_ins(&head, 0, &timer);

	if (gp_timer_queue_process(&head, 10) != 1) {
		tst_msg("Wrong number of timers procesed");
		return TST_FAILED;
	}

	if (gp_timer_queue_size(head) != 1) {
		tst_msg("Timer was not re-inserted from a callback!");
		return TST_FAILED;
	}

	if (timer.expires != 30) {
		tst_msg("Timer expiration not set correctly got %u expected 30",
			(unsigned int)timer.expires);
		return TST_FAILED;
	}

	return TST_PASSED;
}

static uint32_t callback_call_rem(gp_timer *self)
{
	gp_timer **head = self->priv;

	gp_timer_queue_rem(head, self);

	return 0;
}

static uint32_t callback_call_rem_ins(gp_timer *self)
{
	gp_timer **head = self->priv;

	gp_timer_queue_rem(head, self);
	self->expires = 10;
	gp_timer_queue_ins(head, 20, self);

	return 0;
}

static uint32_t callback_call_rem_ins_rem(gp_timer *self)
{
	gp_timer **head = self->priv;

	gp_timer_queue_rem(head, self);
	self->expires = 10;
	gp_timer_queue_ins(head, 20, self);
	gp_timer_queue_rem(head, self);

	return 0;
}

static uint32_t callback_call_rem_rem(gp_timer *self)
{
	gp_timer **head = self->priv;

	gp_timer_queue_rem(head, self);
	gp_timer_queue_rem(head, self);

	return 0;
}

const struct tst_suite tst_suite = {
	.suite_name = "Timer Testsuite",
	.tests = {
		{.name = "Callback is called",
		 .tst_fn = callback_is_called},
		{.name = "Call process with NULL head",
		 .tst_fn = process_with_NULL_head},
		{.name = "Expirations are sorted",
		 .tst_fn = expirations_sorted},
		{.name = "Expirations are sorted + reschedulle",
		 .tst_fn = periodic_timers},
		{.name = "Removal regression",
		 .tst_fn = rem_regression},
		{.name = "Zero reschedulle time from cb",
		 .tst_fn = reschedulle_now},
		{.name = "Call rem from cb",
		 .tst_fn = call_rem_from_cb,
		 .data = callback_call_rem},
		{.name = "Call rem ins from cb",
		 .tst_fn = call_rem_ins_from_cb,
		 .data = callback_call_rem_ins},
		{.name = "Call rem rem from cb",
		 .tst_fn = call_rem_from_cb,
		 .data = callback_call_rem_rem},
		{.name = "Call rem ins rem from cb",
		 .tst_fn = call_rem_from_cb,
		 .data = callback_call_rem_ins_rem},
		{.name = NULL},
	}
};
