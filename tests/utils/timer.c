// SPDX-License-Identifier: GPL-2.1-or-later
/*
 * Copyright (C) 2009-2023 Cyril Hrubis <metan@ucw.cz>
 */

/*

  Test for Timers code.

 */

#include <string.h>
#include <stdlib.h>
#include <inttypes.h>
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
	gp_timer_queue head = {};
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

	if (gp_timer_queue_size(&head)) {
		tst_msg("Timer queue is not empty after stopped timer");
		fail++;
	}

	if (fail)
		return TST_FAILED;

	return TST_PASSED;
}

#define MAX 1000000

static int monotonicity_failed = 0;

static uint32_t callback_check_monotonicity(gp_timer *self)
{
	uint64_t *prev_expires = self->priv;

	if (self->expires < *prev_expires) {
		if (!monotonicity_failed)
			tst_msg("Wrong order of expirations detected");
		monotonicity_failed = 1;
	}

	*prev_expires = self->expires;

	return GP_TIMER_STOP;
}

static int expirations_sorted(void)
{
	gp_timer_queue head = {};
	gp_timer *timers = calloc(MAX, sizeof(gp_timer));
	int i;
	uint64_t prev_expires = 0;

	for (i = 0; i < MAX; i++) {
		timers[i].expires = random();
		timers[i].callback = callback_check_monotonicity;
		timers[i].priv = &prev_expires;
		timers[i].id = "Test";
		gp_timer_queue_ins(&head, 0, &timers[i]);
	}

	prev_expires = gp_timer_queue_first(&head)->expires;

	for (i = 0; i < MAX; i++) {
		gp_timer *first = gp_timer_queue_first(&head);

		gp_timer_queue_process(&head, first ? first->expires : 0);
	}

	if (monotonicity_failed)
		return TST_FAILED;

	if (gp_timer_queue_size(&head)) {
		tst_msg("Some timers rescheduled?!");
		return TST_FAILED;
	}

	return TST_PASSED;
}

static int process_with_NULL_head(void)
{
	gp_timer_queue head = {};

	if (gp_timer_queue_process(&head, 1024)) {
		tst_msg("gp_timer_queue_process returned non-zero");
		return TST_FAILED;
	}

	return TST_PASSED;
}

static uint32_t callback_reschedule(gp_timer *self)
{
	self->priv = (void*)1;

	return self->period;
}

/*
 * Test that periodic timers are rescheduled
 */
static int periodic_timers(void)
{
	GP_TIMER_DECLARE(timer1, 10, 10, "Test1", callback_reschedule, NULL);
	GP_TIMER_DECLARE(timer2, 20, 20, "Test2", callback_reschedule, NULL);
	gp_timer_queue head = {};
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

	if (timer2.priv) {
		tst_msg("Timer2 callback was called");
		fail++;
	}

	/* check that there are two timers in the queue */
	if (gp_timer_queue_size(&head) != 2) {
		tst_msg("Queue head has wrong number of children %u", gp_timer_queue_size(&head));
		fail++;
	}

	if (timer1.expires != 30) {
		tst_msg("Timer1 rescheduled at wrong time %"PRIu64" expected 30", timer1.expires);
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

	if (timer1.expires != 40) {
		tst_msg("Timer1 rescheduled at wrong time %"PRIu64" expected 40", timer1.expires);
		fail++;
	}

	if (!timer2.priv) {
		tst_msg("Timer2 callback was not called");
		fail++;
	}

	if (timer2.expires != 50) {
		tst_msg("Timer2 rescheduled at wrong time %"PRIu64" expected 50", timer2.expires);
		fail++;
	}

	/* check that there are two timers in the queue */
	if (gp_timer_queue_size(&head) != 2) {
		tst_msg("Queue has %u timers, expected 2", gp_timer_queue_size(&head));
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
	gp_timer_queue head = {};

	gp_timer_queue_ins(&head, 10, &timer1);
	gp_timer_queue_ins(&head, 10, &timer2);

	gp_timer_queue_rem(&head, &timer2);

	if (gp_timer_queue_size(&head) != 1) {
		tst_msg("Failed to remove timer");
		return TST_FAILED;
	}

	tst_msg("Timer removed correctly");

	return TST_PASSED;
}

static uint32_t callback_reschedule_now(gp_timer *self)
{
	(void) self;
	return 0;
}

/*
 * Make sure that timer with zero expiration does not cause infinite loop in
 * gp_timer_queue_process()
 */
static int reschedule_now(void)
{
	gp_timer_queue head = {};

	GP_TIMER_DECLARE(timer, 0, 0, "Test", callback_reschedule_now, NULL);

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
	gp_timer_queue head = {};

	GP_TIMER_DECLARE(timer, 0, 10, "Test", callback, &head);

	gp_timer_queue_ins(&head, 0, &timer);

	if (gp_timer_queue_process(&head, 10) != 1) {
		tst_msg("Wrong number of timers procesed");
		return TST_FAILED;
	}

	if (gp_timer_queue_size(&head)) {
		tst_msg("Timer was not removed from a callback!");
		return TST_FAILED;
	}

	return TST_PASSED;
}

static int call_rem_ins_from_cb(void *callback)
{
	gp_timer_queue head = {};

	GP_TIMER_DECLARE(timer, 0, 0, "Test", callback, &head);

	gp_timer_queue_ins(&head, 0, &timer);

	if (gp_timer_queue_process(&head, 10) != 1) {
		tst_msg("Wrong number of timers procesed");
		return TST_FAILED;
	}

	if (gp_timer_queue_size(&head) != 1) {
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
	gp_timer_queue *head = self->priv;

	gp_timer_queue_rem(head, self);

	return 0;
}

static uint32_t callback_call_rem_ins(gp_timer *self)
{
	gp_timer_queue *head = self->priv;

	gp_timer_queue_rem(head, self);
	self->expires = 10;
	gp_timer_queue_ins(head, 20, self);

	return 0;
}

static uint32_t callback_call_rem_ins_rem(gp_timer *self)
{
	gp_timer_queue *head = self->priv;

	gp_timer_queue_rem(head, self);
	self->expires = 10;
	gp_timer_queue_ins(head, 20, self);
	gp_timer_queue_rem(head, self);

	return 0;
}

static uint32_t callback_call_rem_rem(gp_timer *self)
{
	gp_timer_queue *head = self->priv;

	gp_timer_queue_rem(head, self);
	gp_timer_queue_rem(head, self);

	return 0;
}

static uint32_t callback_stop(gp_timer *self)
{
	(void) self;

	return GP_TIMER_STOP;
}

struct ins_other_data {
	gp_timer_queue *queue;
	gp_timer second;
	int first_fires;
	int second_fires;
};

static uint32_t ins_other_second_cb(gp_timer *self)
{
	struct ins_other_data *data = self->priv;

	data->second_fires++;

	return GP_TIMER_STOP;
}

static uint32_t ins_other_first_cb(gp_timer *self)
{
	struct ins_other_data *data = self->priv;

	data->first_fires++;

	data->second.expires = 100;
	data->second.id = "Second";
	data->second.callback = ins_other_second_cb;
	data->second.priv = data;

	gp_timer_queue_ins(data->queue, 0, &data->second);

	return GP_TIMER_STOP;
}

static int ins_other_from_cb(void)
{
	gp_timer_queue head = {};
	struct ins_other_data data = {.queue = &head};

	GP_TIMER_DECLARE(first, 0, 0, "First", ins_other_first_cb, &data);

	gp_timer_queue_ins(&head, 0, &first);

	if (gp_timer_queue_process(&head, 10) != 1) {
		tst_msg("Wrong number of timers processed");
		return TST_FAILED;
	}

	if (gp_timer_queue_size(&head) != 1) {
		tst_msg("Timer inserted from a callback was lost, queue size %u",
		        gp_timer_queue_size(&head));
		return TST_FAILED;
	}

	if (gp_timer_queue_first(&head) != &data.second) {
		tst_msg("Wrong timer left in the queue");
		return TST_FAILED;
	}

	/* The stopped timer must not still be reachable and fire again. */
	if (gp_timer_queue_process(&head, 200) != 1) {
		tst_msg("Wrong number of timers processed in the second run");
		return TST_FAILED;
	}

	if (data.first_fires != 1) {
		tst_msg("First timer fired %i times, expected once",
		        data.first_fires);
		return TST_FAILED;
	}

	if (data.second_fires != 1) {
		tst_msg("Second timer fired %i times, expected once",
		        data.second_fires);
		return TST_FAILED;
	}

	if (gp_timer_queue_size(&head) != 0) {
		tst_msg("Queue not empty at the end, size %u",
		        gp_timer_queue_size(&head));
		return TST_FAILED;
	}

	return TST_PASSED;
}

static int rem_clears_expires(void)
{
	gp_timer_queue head = {};

	GP_TIMER_DECLARE(timer, 10, 10, "Test", callback_stop, NULL);

	gp_timer_queue_ins(&head, 0, &timer);

	gp_timer_queue_rem(&head, &timer);

	if (timer.expires) {
		tst_msg("rem haven't cleared expires");
		return TST_FAILED;
	}

	gp_timer_queue_ins(&head, 0, &timer);

	gp_timer_queue_process(&head, 20);

	if (timer.expires) {
		tst_msg("GP_TIMER_STOP from callback haven't cleared expires");
		return TST_FAILED;
	}

	return TST_PASSED;
}

struct rem_res_data {
	gp_timer_queue *head;
	gp_timer *rem_parked_victim;
	int rem_parked_stopped;
};

static uint32_t callback_reschedule_0(gp_timer *self)
{
	(void) self;

	/*
	 * Zero timeout means that the timer has to go to the reschedule list
	 * first before the current batch is finished.
	 */
	return 0;
}

static void rem_parked_stopped_cb(gp_timer *self)
{
	struct rem_res_data *data = self->priv;

	data->rem_parked_stopped++;
}

static uint32_t callback_rem_parked(gp_timer *self)
{
	struct rem_res_data *data = self->priv;

	gp_timer_queue_rem(data->head, data->rem_parked_victim);

	data->rem_parked_victim->expires = 10;

	gp_timer_queue_ins(data->head, 10, data->rem_parked_victim);

	gp_timer_queue_rem(data->head, data->rem_parked_victim);

	return GP_TIMER_STOP;
}

static int rem_rescheduled_from_cb(void)
{
	gp_timer_queue head = {};

	GP_TIMER_DECLARE(first, 5, 10, "First", callback_reschedule_0, NULL);
	GP_TIMER_DECLARE(second, 10, 0, "Second", callback_rem_parked, NULL);

	struct rem_res_data data = {
		.head = &head,
		.rem_parked_victim = &first,
	};

	first.stopped = rem_parked_stopped_cb;

	first.priv = &data;
	second.priv = &data;

	gp_timer_queue_ins(&head, 0, &first);
	gp_timer_queue_ins(&head, 0, &second);

	if (gp_timer_queue_process(&head, 10) != 2) {
		tst_msg("Wrong number of timers processed");
		return TST_FAILED;
	}

	if (gp_timer_queue_size(&head)) {
		tst_msg("Removed timer was re-inserted anyway");
		return TST_FAILED;
	}

	if (data.rem_parked_stopped != 1) {
		tst_msg("stopped() was not called for the removed timer");
		return TST_FAILED;
	}

	if (gp_timer_is_running(&first)) {
		tst_msg("Removed timer still claims to be running");
		return TST_FAILED;
	}

	/* The queue must still be usable — a corrupted one loops forever. */
	GP_TIMER_DECLARE(fill, 100, 0, "Fill", callback_reschedule_0, NULL);
	gp_timer_queue_ins(&head, 0, &fill);

	if (gp_timer_queue_size(&head) != 1) {
		tst_msg("Queue is not usable after the removal");
		return TST_FAILED;
	}

	return TST_PASSED;
}

struct order_log {
	unsigned int len;
	char ids[16];
};

static void order_log_id(struct order_log *log, const char *id)
{
	if (log->len + 1 < sizeof(log->ids))
		log->ids[log->len++] = id[0];
}

static uint32_t callback_log_id(gp_timer *self)
{
	order_log_id(self->priv, self->id);

	return GP_TIMER_STOP;
}

static int same_expiration_runs_in_insertion_order(void)
{
	int fail = 0;

	const char *want[] = {"ABCD", "DCBA"};
	unsigned int i;

	for (i = 0; i < 2; i++) {
		struct order_log log = {};
		GP_TIMER_DECLARE(ta, 10, 0, "A", callback_log_id, &log);
		GP_TIMER_DECLARE(tb, 10, 0, "B", callback_log_id, &log);
		GP_TIMER_DECLARE(tc, 10, 0, "C", callback_log_id, &log);
		GP_TIMER_DECLARE(td, 10, 0, "D", callback_log_id, &log);
		gp_timer_queue head = {};
		gp_timer *fwd[] = {&ta, &tb, &tc, &td};
		gp_timer *rev[] = {&td, &tc, &tb, &ta};
		gp_timer **ins = i ? rev : fwd;
		unsigned int j;

		for (j = 0; j < 4; j++)
			gp_timer_queue_ins(&head, 0, ins[j]);

		if (gp_timer_queue_process(&head, 10) != 4) {
			tst_msg("Not all four timers expired");
			fail++;
		}

		if (strcmp(log.ids, want[i])) {
			tst_msg("Inserted %s ran as %s, expected %s",
			        want[i], log.ids, want[i]);
			fail++;
		}
	}

	if (fail)
		return TST_FAILED;

	return TST_PASSED;
}

/*
 * A periodic timer that reschedules itself goes to the BACK of the
 * millisecond it lands in.
 */
static uint32_t callback_log_id_periodic(gp_timer *self)
{
	order_log_id(self->priv, self->id);

	return 10;
}

static int reschedule_goes_to_the_back(void)
{
	struct order_log log = {};
	GP_TIMER_DECLARE(tp, 10, 10, "P", callback_log_id_periodic, &log);
	GP_TIMER_DECLARE(tq, 20, 0, "Q", callback_log_id, &log);
	gp_timer_queue head = {};

	gp_timer_queue_ins(&head, 0, &tp);
	gp_timer_queue_ins(&head, 0, &tq);

	/* P at 10; then P re-armed for 20, where Q already waits. */
	gp_timer_queue_process(&head, 10);
	gp_timer_queue_process(&head, 20);

	/* Q was queued first for 20, so Q runs before the re-armed P. */
	if (strcmp(log.ids, "PQP")) {
		tst_msg("Ran as %s, expected PQP", log.ids);
		return TST_FAILED;
	}

	return TST_PASSED;
}

/*
 * Two timers with the same period keep their order, tick after tick.
 */
static int reschedule_keeps_order(void)
{
	struct order_log log = {};
	GP_TIMER_DECLARE(tp, 10, 10, "P", callback_log_id_periodic, &log);
	GP_TIMER_DECLARE(tr, 10, 10, "R", callback_log_id_periodic, &log);
	gp_timer_queue head = {};
	uint64_t now;

	gp_timer_queue_ins(&head, 0, &tp);
	gp_timer_queue_ins(&head, 0, &tr);

	for (now = 10; now <= 50; now += 10)
		gp_timer_queue_process(&head, now);

	if (strcmp(log.ids, "PRPRPRPRPR")) {
		tst_msg("Ran as %s, expected PRPRPRPRPR", log.ids);
		return TST_FAILED;
	}

	return TST_PASSED;
}

/*
 * A timer that is rescheduled runs before one that a LATER callback in the
 * same batch inserts for the same moment.
 */
struct ins_late_data {
	struct order_log *log;
	gp_timer_queue *queue;
	gp_timer late;
};

static uint32_t callback_ins_late(gp_timer *self)
{
	struct ins_late_data *data = self->priv;

	order_log_id(data->log, self->id);

	data->late.expires = 10;
	data->late.id = "Q";
	data->late.callback = callback_log_id;
	data->late.priv = data->log;

	gp_timer_queue_ins(data->queue, 10, &data->late);

	return GP_TIMER_STOP;
}

static int reschedule_before_later_ins(void)
{
	struct order_log log = {};
	gp_timer_queue head = {};
	struct ins_late_data data = {.log = &log, .queue = &head};
	GP_TIMER_DECLARE(tp, 10, 10, "P", callback_log_id_periodic, &log);
	GP_TIMER_DECLARE(tr, 10, 0, "R", callback_ins_late, &data);

	gp_timer_queue_ins(&head, 0, &tp);
	gp_timer_queue_ins(&head, 0, &tr);

	/* P is rescheduled for 20 while R, running after it, sets Q for 20. */
	gp_timer_queue_process(&head, 10);
	gp_timer_queue_process(&head, 20);

	if (strcmp(log.ids, "PRPQ")) {
		tst_msg("Ran as %s, expected PRPQ", log.ids);
		return TST_FAILED;
	}

	return TST_PASSED;
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
		{.name = "Expirations are sorted + reschedule",
		 .tst_fn = periodic_timers},
		{.name = "Removal regression",
		 .tst_fn = rem_regression},
		{.name = "Removal clears expires",
		 .tst_fn = rem_clears_expires},
		{.name = "Zero reschedule time from cb",
		 .tst_fn = reschedule_now},
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
		{.name = "Ins another timer from cb",
		 .tst_fn = ins_other_from_cb},
		{.name = "Rem a rescheduled timer from another cb",
		 .tst_fn = rem_rescheduled_from_cb},
		{.name = "Same expiration runs in insertion order",
		 .tst_fn = same_expiration_runs_in_insertion_order},
		{.name = "Reschedule goes to the back of its ms",
		 .tst_fn = reschedule_goes_to_the_back},
		{.name = "Reschedule keeps the batch order",
		 .tst_fn = reschedule_keeps_order},
		{.name = "Reschedule runs before a later insert",
		 .tst_fn = reschedule_before_later_ins},
		{.name = NULL},
	}
};
