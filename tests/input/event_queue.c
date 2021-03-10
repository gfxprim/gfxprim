// SPDX-License-Identifier: GPL-2.1-or-later
/*
 * Copyright (C) 2009-2021 Cyril Hrubis <metan@ucw.cz>
 */

#include <string.h>
#include <stdlib.h>
#include <input/gp_input.h>

#include "tst_test.h"

static int get_pointer_preserved(void)
{
	unsigned int i;
	gp_event_queue queue;
	gp_event *ev;

	gp_event_queue_init(&queue, 1, 1, 0);

	gp_event_queue_push_key(&queue, GP_KEY_ENTER, GP_EV_KEY_DOWN, NULL);

	ev = gp_event_queue_get(&queue);

	if (!ev) {
		tst_msg("No event returned");
		return TST_FAILED;
	}

	if (ev->key.key != GP_KEY_ENTER) {
		tst_msg("Got wrong event");
		return TST_FAILED;
	}

	for (i = 0; i < GP_EVENT_QUEUE_SIZE; i++)
		gp_event_queue_push_key(&queue, GP_KEY_SPACE, GP_EV_KEY_DOWN, NULL);

	if (ev->key.key != GP_KEY_ENTER) {
		tst_msg("Event rewritten");
		return TST_FAILED;
	}

	return TST_SUCCESS;
}

static int peek_test(void)
{
	unsigned int i;
	gp_event_queue queue;
	gp_event *ev;

	gp_event_queue_init(&queue, 1, 1, 0);

	gp_event_queue_push_key(&queue, GP_KEY_A, GP_EV_KEY_DOWN, NULL);
	gp_event_queue_push_key(&queue, GP_KEY_ENTER, GP_EV_KEY_DOWN, NULL);

	for (i = 0; i < GP_EVENT_QUEUE_SIZE-2; i++)
		gp_event_queue_push_key(&queue, GP_KEY_SPACE, GP_EV_KEY_DOWN, NULL);

	if (gp_event_queue_key_pressed(&queue, GP_KEY_ENTER)) {
		tst_msg("Key enter is pressed");
		return TST_FAILED;
	}

	gp_event_queue_get(&queue);

	ev = gp_event_queue_peek(&queue);

	if (!ev) {
		tst_msg("No event returned");
		return TST_FAILED;
	}

	if (ev->key.key != GP_KEY_ENTER) {
		tst_msg("Wrong event peeked!");
		return TST_FAILED;
	}

	if (gp_event_queue_key_pressed(&queue, GP_KEY_ENTER)) {
		tst_msg("Key enter is pressed after peek");
		return TST_FAILED;
	}

	return TST_SUCCESS;
}

static int key_state_test(void)
{
	gp_event_queue queue;
	gp_event *ev;
	int fail = 0;

	gp_event_queue_init(&queue, 1, 1, 0);

	gp_event_queue_push_key(&queue, GP_KEY_A, GP_EV_KEY_DOWN, NULL);
	gp_event_queue_push_key(&queue, GP_KEY_A, GP_EV_KEY_UP, NULL);
	gp_event_queue_push_key(&queue, GP_KEY_ENTER, GP_EV_KEY_DOWN, NULL);
	gp_event_queue_push_key(&queue, GP_KEY_SPACE, GP_EV_KEY_DOWN, NULL);
	gp_event_queue_push_key(&queue, GP_KEY_ENTER, GP_EV_KEY_UP, NULL);
	gp_event_queue_push_key(&queue, GP_KEY_SPACE, GP_EV_KEY_REPEAT, NULL);
	gp_event_queue_push_key(&queue, GP_KEY_SPACE, GP_EV_KEY_UP, NULL);

	while ((ev = gp_event_queue_get(&queue))) {
		switch (ev->code) {
		case GP_EV_KEY_UP:
			if (gp_event_queue_key_pressed(&queue, ev->key.key)) {
				tst_msg("Key %s pressed after keyup",
				        gp_event_key_name(ev->key.key));
				fail++;
			}
		break;
		case GP_EV_KEY_DOWN:
			if (!gp_event_queue_key_pressed(&queue, ev->key.key)) {
				tst_msg("Key %s released after keydown",
				        gp_event_key_name(ev->key.key));
				fail++;
			}
		break;
		}
	}

	if (fail)
		return TST_FAILED;

	return TST_SUCCESS;
}

static int queue_init_test(void)
{
	unsigned int i;
	gp_event_queue queue;

	gp_event_queue_init(&queue, 1, 1, 0);

	if (gp_event_queue_events(&queue)) {
		tst_msg("Events queued after init");
		return TST_FAILED;
	}

	if (gp_event_queue_get(&queue)) {
		tst_msg("Event get returned event after init");
		return TST_FAILED;
	}

	for (i = 0; i < 1000; i++) {
		if (gp_event_queue_key_pressed(&queue, i)) {
			tst_msg("Key %u pressed after init", i);
			return TST_FAILED;
		}
	}

	return TST_SUCCESS;
}

const struct tst_suite tst_suite = {
	.suite_name = "Event Queue Testsuite",
	.tests = {
		{.name = "Queue init",
		 .tst_fn = queue_init_test},
		{.name = "Event peek",
		 .tst_fn = peek_test},
		{.name = "Key state",
		 .tst_fn = key_state_test},
		{.name = "Pointer from get is preserved",
		 .tst_fn = get_pointer_preserved},
		{.name = NULL},
	}
};
