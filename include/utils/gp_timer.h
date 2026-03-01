// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2009-2023 Cyril Hrubis <metan@ucw.cz>
 */

/**
 * @file gp_timer.h
 * @brief Timers and timer queue implementation.
 */

#ifndef UTILS_GP_TIMER_H
#define UTILS_GP_TIMER_H

#include <stdint.h>
#include <utils/gp_types.h>
#include <input/gp_types.h>

/**
 * @brief Return this value from the callback to stop a timer.
 */
#define GP_TIMER_STOP UINT32_MAX

/**
 * @brief A timer.
 */
struct gp_timer {
	union {
		gp_heap_head heap;
		gp_timer *next;
	};

	/** @brief Initial xpiration time, set by user, modified by the queue */
	uint64_t expires;

	/** @brief Timer name showed in debug messages */
	const char *id;

	/**
	 * @brief User variable may be used to store the timer period.
	 *
	 * This field is not used by the timer code and is usually returned
	 * from the callback to reschedule periodic timers.
	 */
	uint32_t period;

	/** @brief Set if timer is inserted into a queue */
	uint32_t running:1;
	/** @brief Set during the run of the timer callback */
	uint32_t in_callback:1;
	/** @brief Set if timer was rescheduled from callback */
	uint32_t res_in_callback:1;
	/**
	 * @brief Set if gp_timer_free() was called when timer was running.
	 *
	 * This defferes freeing the timer memory afte the timer is stopped.
	 */
	uint32_t free_on_stop:1;



	/** @brief Library private pointer. Do not touch! */
	void *_priv;

	/**
	 * @brief Timer callback
	 *
	 * Unless GP_TIMER_STOP is returned the timer is rescheduled to expire
	 * return value from now.
	 */
	uint32_t (*callback)(struct gp_timer *self);

	/** @brief A user private pointer */
	void *priv;

	/**
	 * @brief An optional function to cleanup when timer is stopped.
	 *
	 * When non-NULL this function is called when timer is stopped. Timer
	 * can be either stopped when gp_timer::callback returns #GP_TIMER_STOP
	 * or when a timer was removed from the queue.
	 *
	 * It is safe to free dynamically alocated timers from this callback.
	 */
	void (*stopped)(gp_timer *self);

	char data[];
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
 * @brief Returns if timer is running.
 *
 * A timer is running if it's inserted into a timer queue.
 *
 * @param timer A timer.
 * @return Non-zero if timer is running, zero otherwise.
 */
static inline int gp_timer_is_running(const gp_timer *timer)
{
	return timer->running;
}

/**
 * @brief Prints the structrue of binary heap into stdout, only for debugging.
 *
 * @param queue A timer queue.
 */
void gp_timer_queue_dump(const gp_timer *queue);

/**
 * @brief Inserts timer into the timer priority queue.
 *
 * If timer is already running nothing is done. In order to reschedule a timer
 * it has to be removed from the queue first.
 *
 * It's safe to call insert from the timer callback.
 *
 * If timer was removed then inserted from the timer callback the timer is
 * rescheduled at now + timer->expired time and the return value from the
 * callback is discarded.
 *
 * @param queue A timer queue.
 * @param now A timestamp, usually obtained by calling gp_time_stamp().
 * @param timer A timer to insert.
 */
void gp_timer_queue_ins(gp_timer **queue, uint64_t now, gp_timer *timer);

/**
 * @brief Removes timer from timer queue.
 *
 * It's safe to call remove from the timer callback.
 *
 * If called from the timer callback the timer is stopped and the return value
 * from the callback is discarded.
 *
 * @param queue A timer queue.
 * @param timer A timer to insert.
 */
void gp_timer_queue_rem(gp_timer **queue, gp_timer *timer);

/**
 * @brief Processes queue, all timers with expires <= now are processed.
 *
 * It's possible to reschedule a timer with expires set to 0, which will
 * process the timer on the next process call.
 *
 * @param queue A timer queue.
 * @param now A timestamp, usually obtained by calling gp_time_stamp().
 * @return Number of timers processed.
 */
int gp_timer_queue_process(gp_timer **queue, uint64_t now);

/**
 * @brief Returns size of the queue, i.e. number of timers.
 *
 * @param queue A timer queue.
 * @return A Number of timers in the queue.
 */
static inline unsigned int gp_timer_queue_size(const gp_timer *queue)
{
	return queue ? queue->heap.children + 1 : 0;
}

/**
 * @brief Allocates a new timer.
 *
 * Allocates and intializes new timer.
 *
 * @param expires_ms Initial expiration for the timer. The timer starts
 *                   counting once it's inserted into a timer queue.
 * @param period_ms Sets the timer period field.
 * @param id A timer id string, the string is copied during the allocation.
 * @param callback A timer callback.
 * @param priv A user private pointer.
 */
gp_timer *gp_timer_alloc(uint32_t expires_ms, uint32_t period_ms, const char *id,
                         uint32_t (*callback)(gp_timer *), void *priv);

/**
 * @brief Frees timer allocated by gp_timer_alloc().
 *
 * When timer is inserted into a queue the free is deffered until the timer is
 * stopped. Calling gp_timer_free() on a timer that has been inserted into a
 * queue hence transfers the ownership to the queue itself.
 *
 * @param self A timer allocated by gp_timer_alloc().
 */
void gp_timer_free(gp_timer *self);

#endif /* UTILS_GP_TIMER_H */
