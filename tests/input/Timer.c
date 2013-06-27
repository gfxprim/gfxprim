/*****************************************************************************
 * This file is part of gfxprim library.                                     *
 *                                                                           *
 * Gfxprim is free software; you can redistribute it and/or                  *
 * modify it under the terms of the GNU Lesser General Public                *
 * License as published by the Free Software Foundation; either              *
 * version 2.1 of the License, or (at your option) any later version.        *
 *                                                                           *
 * Gfxprim is distributed in the hope that it will be useful,                *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of            *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU         *
 * Lesser General Public License for more details.                           *
 *                                                                           *
 * You should have received a copy of the GNU Lesser General Public          *
 * License along with gfxprim; if not, write to the Free Software            *
 * Foundation, Inc., 51 Franklin Street, Fifth Floor,                        *
 * Boston, MA  02110-1301  USA                                               *
 *                                                                           *
 * Copyright (C) 2009-2013 Cyril Hrubis <metan@ucw.cz>                       *
 *                                                                           *
 *****************************************************************************/

/*

  Test for Timers code.

 */

#include <string.h>
#include <stdlib.h>
#include <input/GP_Timer.h>

#include "tst_test.h"

static uint32_t callback_set_priv(GP_Timer *self)
{
	self->priv = (void*)1;

	return 0;
}

static int callback_is_called(void)
{
	GP_TIMER_DECLARE(timer, 10, 0, "Timer", callback_set_priv, NULL);
	GP_Timer *head = NULL;
	int fail = 0;

	GP_TimerQueueInsert(&head, 10, &timer);

	/* Now call process before the timer expiration */
	if (GP_TimerQueueProcess(&head, 10)) {
		tst_msg("GP_TimerQueueProcess() reported non-zero");
		fail++;
	}

	if (timer.priv) {
		tst_msg("Callback was called");
		fail++;
	}

	/* Now call process after the expiration time */
	if (GP_TimerQueueProcess(&head, 30) != 1) {
		tst_msg("GP_TimerQueueProcess() reported wrong number");
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

static uint32_t callback_check_monotonicity(GP_Timer *self)
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
	GP_Timer *head = NULL;
	GP_Timer timers[MAX];
	int i;
	uint64_t expires;

	for (i = 0; i < MAX; i++) {
		timers[i].expires = random();
		timers[i].period = 0;
		timers[i].Callback = callback_check_monotonicity;
		timers[i].priv = &expires;
		strcpy(timers[i].id, "Timer");
		GP_TimerQueueInsert(&head, 0, &timers[i]);
	}

	prev_expires = head->expires;

	for (i = 0; i < MAX; i++)
		GP_TimerQueueProcess(&head, head ? head->expires : 0);

	if (monotonicity_failed)
		return TST_FAILED;

	return TST_SUCCESS;
}

static int process_with_NULL_head(void)
{
	GP_Timer *head = NULL;

	if (GP_TimerQueueProcess(&head, 1024)) {
		tst_msg("GP_TimerQueueProcess returned non-zero");
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
	GP_Timer *head = NULL;
	int fail = 0;

	GP_TimerQueueInsert(&head, 10, &timer1);
	GP_TimerQueueInsert(&head, 10, &timer2);

	/* Make timer1 expire */
	if (GP_TimerQueueProcess(&head, 20) != 1) {
		tst_msg("GP_TimerQueueProcess() reported wrong number");
		fail++;
	}

	if (!timer1.priv) {
		tst_msg("Timer1 callback was not called");
		fail++;
	}

	/* check that there are two timers in the queue */
	if (head->sons != 1) {
		tst_msg("Queue head has wrong number of sons %u", head->sons);
		fail++;
	}

	timer1.priv = NULL;

	/* Make both timers expire */
	if (GP_TimerQueueProcess(&head, 30) != 2) {
		tst_msg("GP_TimerQueueProcess() reported wrong number");
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
	if (head->sons != 1) {
		tst_msg("Queue head has wrong number of sons %u", head->sons);
		fail++;
	}

	if (fail)
		return TST_FAILED;

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
		{.name = NULL},
	}
};
