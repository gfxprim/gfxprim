// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2009-2013 Cyril Hrubis <metan@ucw.cz>
 */

/*

  Timers and timer queue implementation.

 */

#ifndef INPUT_GP_TIMER_H
#define INPUT_GP_TIMER_H

#include <stdint.h>
#include <utils/gp_types.h>
#include <input/gp_types.h>

/*
 * Return this value from the callback to stop a periodical timer.
 */
#define GP_TIMER_PERIOD_STOP UINT32_MAX

struct gp_timer {
	gp_heap_head heap;

	/* Expiration time */
	uint64_t expires;

	/* Timer id, showed in debug messages */
	const char *id;

	/*
	 * If not zero return value from callback is ignored and
	 * timer is rescheduled each time it expires.
	 */
	uint32_t period;
	/* Set if timer is inserted into a queue */
	uint32_t running:1;
	/* Used to signal that ins or rem was called from callback */
	uint32_t in_callback:1;

	/* Do not touch */
	void *_priv;

	/*
	 * Timer callback
	 *
	 * If non-zero is returned, the timer is rescheduled to expire
	 * return value from now.
	 */
	uint32_t (*callback)(struct gp_timer *self);
	void *priv;
};

#define GP_TIMER_DECLARE(name, texpires, tperiod, tid, tcallback, tpriv) \
	gp_timer name = { \
		.expires = texpires, \
		.period = tperiod, \
		.id = tid, \
		.callback = tcallback, \
		.priv = tpriv \
	}
/**
 * Returns if timer is running, i.e. inserted into a timer queue.
 *
 * @timer A timer.
 * @return Non-zero if timer is running, zero otherwise.
 */
static inline int gp_timer_running(gp_timer *timer)
{
	return timer->running;
}

/*
 * Prints the structrue of binary heap into stdout, only for debugging.
 */
void gp_timer_queue_dump(gp_timer *queue);

/**
 * Inserts timer into the timer priority queue.
 *
 * If timer is already running nothing is done.
 *
 * @queue A timer queue.
 * @now A timestamp, usually obtained by calling gp_time_stamp().
 * @timer A timer to insert.
 */
void gp_timer_queue_insert(gp_timer **queue, uint64_t now, gp_timer *timer);

/*
 * Removes timer from timer queue.
 *
 * This operation (in contrast with insert and process) runs in O(n) time.
 */
void gp_timer_queue_remove(gp_timer **queue, gp_timer *timer);

/*
 * Processes queue, all timers with expires <= now are processed.
 *
 * Returns number of timers processed.
 */
int gp_timer_queue_process(gp_timer **queue, uint64_t now);

/*
 * Returns size of the queue, i.e. number of timers.
 */
static inline unsigned int gp_timer_queue_size(gp_timer *queue)
{
	return queue ? queue->heap.children + 1 : 0;
}

#endif /* INPUT_GP_TIMER_H */
