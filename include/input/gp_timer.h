// SPDX-License-Identifier: GPL-2.1-or-later
/*
 * Copyright (C) 2009-2013 Cyril Hrubis <metan@ucw.cz>
 */

/*

  Timers and timer queue implementation.

 */

#ifndef INPUT_GP_TIMER_H
#define INPUT_GP_TIMER_H

#include <stdint.h>

#include <input/gp_types.h>

struct gp_timer {
	/* Heap pointers and number of sons */
	gp_timer *left;
	gp_timer *right;
	unsigned int sons;

	/* Expiration time */
	uint64_t expires;
	/*
	 * If not zero return value from callback is ignored and
	 * timer is rescheduled each time it expires.
	 */
	uint32_t period;

	/* Timer id, showed in debug messages */
	const char *id;

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

/*
 * Prints the structrue of binary heap into stdout, only for debugging.
 */
void gp_timer_queue_dump(gp_timer *queue);

/*
 * Inserts timer into the timer priority queue.
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
	return queue ? queue->sons + 1 : 0;
}

#endif /* INPUT_GP_TIMER_H */
