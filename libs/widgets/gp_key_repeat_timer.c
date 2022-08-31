//SPDX-License-Identifier: LGPL-2.0-or-later

/*

   Copyright (c) 2014-2020 Cyril Hrubis <metan@ucw.cz>

 */

/*
 * Keboard key press events send KEY_REPEAT events after a while if key is
 * pressed continuously. However there is no such functionality for neither the
 * mouse buttons nor the touchscreen touch events.
 *
 * This file implements a simple event filter that sets up timer and feeds key
 * repeat events for a selected set of keys into an event queue.
 */

#include <stddef.h>
#include <input/gp_timer.h>
#include <input/gp_event_queue.h>
#include <input/gp_time_stamp.h>
#include <widgets/gp_widget_timer.h>

static struct gp_event_queue *event_queue;
static gp_timer **timer_queue;
static uint32_t repeat_delay = 1000;
static uint32_t repeat_period = 100;

void gp_key_repeat_timer_init(gp_event_queue *ev_queue, gp_timer **tmr_queue)
{
	event_queue = ev_queue;
	timer_queue = tmr_queue;
}

static uint32_t btn_repeat_callback(gp_timer *self)
{
	long key = (long)self->priv;

	gp_event_queue_push_key(event_queue, key, GP_EV_KEY_REPEAT, 0);
	return repeat_period;
}

static int key_to_idx(int key)
{
	switch (key) {
	case GP_BTN_LEFT:
		return 0;
	case GP_BTN_TOUCH:
		return 1;
	default:
		return -1;
	}
}

static gp_timer timers[] = {
	{
		.period = 0,
		.callback = btn_repeat_callback,
		.id = "BTN_LEFT repeat timer",
		.priv = (void*)((long)GP_BTN_LEFT)
        },
	{
		.period = 0,
		.callback = btn_repeat_callback,
		.id = "BTN_TOUCH repeat timer",
		.priv = (void*)((long)GP_BTN_TOUCH)
	}
};

static char timer_state[2];

static void start_timer(int key)
{
	int i = key_to_idx(key);

	if (timer_state[i])
		return;

	timers[i].expires = repeat_delay;

	gp_timer_queue_insert(timer_queue, gp_time_stamp(), &timers[i]);

	timer_state[i] = 1;
}

static void stop_timer(int key)
{
	int i = key_to_idx(key);

	if (!timer_state[i])
		return;

	gp_timer_queue_remove(timer_queue, &timers[i]);

	timer_state[i] = 0;
}

void gp_handle_key_repeat_timer(gp_event *ev)
{
	if (ev->type != GP_EV_KEY)
		return;

	if (key_to_idx(ev->key.key) == -1)
		return;

	switch (ev->code) {
	case GP_EV_KEY_UP:
		stop_timer(ev->key.key);
	break;
	case GP_EV_KEY_DOWN:
		start_timer(ev->key.key);
	break;
	}
}
