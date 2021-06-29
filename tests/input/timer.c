// SPDX-License-Identifier: GPL-2.1-or-later
/*
 * Copyright (C) 2009-2013 Cyril Hrubis <metan@ucw.cz>
 */

/*

  Test for Timers code.

 */

#include <string.h>
#include <stdlib.h>
#include <input/gp_timer.h>

#include "tst_test.h"

static uint32_t callback_set_priv(gp_timer *self)
{
	self->priv = (void*)1;

	return 0;
}

static int callback_is_called(void)
{
	GP_TIMER_DECLARE(timer, 10, 0, "Timer", callback_set_priv, NULL);
	gp_timer *head = NULL;
	int fail = 0;

	gp_timer_queue_insert(&head, 10, &timer);

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

	if (fail)
		return TST_FAILED;

	return TST_SUCCESS;
}

#define MAX 2048

static int monotonicity_failed = 0;
static uint64_t prev_expires;

static uint32_t callback_check_monotonicity(gp_timer *self)
{
	if (self->expires < prev_expires) {
		monotonicity_failed = 1;
		tst_msg("Wrong order of expirations detected");
	}

	prev_expires = self->expires;

	return 0;
}

static int expirations_sorted(void)
{
	gp_timer *head = NULL;
	gp_timer timers[MAX];
	int i;
	uint64_t expires;

	for (i = 0; i < MAX; i++) {
		timers[i].expires = random();
		timers[i].period = 0;
		timers[i].callback = callback_check_monotonicity;
		timers[i].priv = &expires;
		timers[i].id = "Timer";
		gp_timer_queue_insert(&head, 0, &timers[i]);
	}

	prev_expires = head->expires;

	for (i = 0; i < MAX; i++)
		gp_timer_queue_process(&head, head ? head->expires : 0);

	if (monotonicity_failed)
		return TST_FAILED;

	return TST_SUCCESS;
}

static int process_with_NULL_head(void)
{
	gp_timer *head = NULL;

	if (gp_timer_queue_process(&head, 1024)) {
		tst_msg("gp_timer_queue_process returned non-zero");
		return TST_FAILED;
	}

	return TST_SUCCESS;
}

/*
 * Test that periodic timers are rescheduled
 */
static int periodic_timers(void)
{
	GP_TIMER_DECLARE(timer1, 0, 10, "Timer1", callback_set_priv, NULL);
	GP_TIMER_DECLARE(timer2, 0, 20, "Timer2", callback_set_priv, NULL);
	gp_timer *head = NULL;
	int fail = 0;

	gp_timer_queue_insert(&head, 10, &timer1);
	gp_timer_queue_insert(&head, 10, &timer2);

	/* Make timer1 expire */
	if (gp_timer_queue_process(&head, 20) != 1) {
		tst_msg("gp_timer_queue_process() reported wrong number");
		fail++;
	}

	if (!timer1.priv) {
		tst_msg("Timer1 callback was not called");
		fail++;
	}

	/* check that there are two timers in the queue */
	if (head->heap.children != 1) {
		tst_msg("Queue head has wrong number of children %u", head->heap.children);
		fail++;
	}

	timer1.priv = NULL;

	/* Make both timers expire */
	if (gp_timer_queue_process(&head, 30) != 2) {
		tst_msg("gp_timer_queue_process() reported wrong number");
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
		tst_msg("Queue head has wrong number of children %u", head->heap.children);
		fail++;
	}

	if (fail)
		return TST_FAILED;

	return TST_SUCCESS;
}

/*
 * Special case where we remove a timer with the highest value, there was a bug
 * in the library heap that caused crash with this.
 */
static int rem_regression(void)
{
	GP_TIMER_DECLARE(timer1, 0, 10, "Timer1", NULL, NULL);
	GP_TIMER_DECLARE(timer2, 0, 20, "Timer2", NULL, NULL);
	gp_timer *head = NULL;

	gp_timer_queue_insert(&head, 10, &timer1);
	gp_timer_queue_insert(&head, 10, &timer2);

	gp_timer_queue_remove(&head, &timer2);

	if (head->heap.children) {
		tst_msg("Failed to remove timer");
		return TST_FAILED;
	}

	tst_msg("Timer removed correctly");

	return TST_SUCCESS;
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
		{.name = "Periodic timers",
		 .tst_fn = periodic_timers},
		{.name = "Removal regression",
		 .tst_fn = rem_regression},
		{.name = NULL},
	}
};
