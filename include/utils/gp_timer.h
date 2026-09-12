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
#include <core/gp_common.h>
#include <utils/gp_types.h>
#include <utils/gp_avl_tree.h>
#include <utils/gp_list.h>
#include <input/gp_types.h>

/**
 * @brief Return this value from the callback to stop a timer.
 */
#define GP_TIMER_STOP UINT32_MAX

/**
 * @brief A timer.
 */
struct gp_timer {
	/**
	 * @brief A queue tree node, valid for a bucket head. Do not touch!
	 *
	 * Timers that expire at the same time share one node in the queue
	 * tree: they are chained in a list and only the first of them, the
	 * bucket head, is in the tree.
	 */
	gp_avl_node avl;

	/**
	 * @brief A link in the bucket, a circular list. Do not touch!
	 *
	 * Timers set for the same moment are expected to run in the order they
	 * were inserted. A timer is pushed to the tail of its bucket, so the
	 * order is the list's and nothing has to be stamped or compared to
	 * keep it.
	 *
	 * The list is circular and anchored at the bucket head.
	 *
	 * Doubles as the link of the reschedule list while a timer waits to
	 * be put back into the queue, see gp_timer::reschedule.
	 */
	gp_dlist_head lh;

	/** @brief Initial expiration time, set by user, modified by the queue */
	uint64_t expires;

	/** @brief Timer name showed in debug messages */
	const char *id;

	/**
	 * @brief User variable may be used to store the timer period.
	 *
	 * This field is not used by the timer code and is usually returned
	 * from the user callback to reschedule periodic timers.
	 */
	uint32_t period;

	/** @brief Set if timer is inserted into a queue */
	uint32_t running:1;
	/**
	 * @brief Set if the timer is the bucket head is inserted into the tree.
	 *
	 * That means gp_timer::avl is inserted into the tree, the avl pointers
	 * are unused if this is not set.
	 */
	uint32_t bucket_head:1;
	/**
	 * @brief Set during the run of the timer callback.
	 */
	uint32_t in_callback:1;
	/**
	 * @brief Set if timer was rescheduled from callback.
	 */
	uint32_t res_in_callback:1;
	/**
	 * @brief Timer was processed and is waiting for rescheduling.
	 *
	 * This is set when timer was processed and is queued for a
	 * rescheduling. If this is set the insert and remove does not modify
	 * any data structures, just update the gp_timer::expires value and
	 * gp_timer::running flag.
	 */
	uint32_t reschedule:1;

	/**
	 * @brief Frees timer when it's stopped.
	 *
	 * This is set automatically if gp_timer_free() was called when timer
	 * was running.
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

/**
 * @brief A helper macro to declare a timer.
 *
 * @param name A variable name.
 * @param texpires An initial timer expirantion.
 * @param tperiod A value to store in the timer::period variable.
 * @param tid A string with a timer name, used for debugging.
 * @param tcallback A timer callback.
 * @param tpriv A value to store in the timer::priv variable.
 */
#define GP_TIMER_DECLARE(name, texpires, tperiod, tid, tcallback, tpriv) \
	gp_timer name = { \
		.expires = texpires, \
		.period = tperiod, \
		.id = tid, \
		.callback = tcallback, \
		.priv = tpriv \
	}

/**
 * @brief A helper to initialize a timer.
 *
 * Clears are fields that are internally used by the library.
 *
 * @param texpires An initial timer expirantion.
 * @param tperiod A value to store in the timer::period variable.
 * @param tid A string with a timer name, used for debugging.
 * @param tcallback A timer callback.
 * @param tpriv A value to store in the timer::priv variable.
 */
static inline void gp_timer_init(gp_timer *self, uint64_t expires,
                                 uint32_t period, const char *id,
				 uint32_t callback(gp_timer *), void *priv)
{
	self->expires = expires;
	self->period = period;
	self->id = id;
	self->callback = callback;
	self->priv = priv;
	self->running = 0;
	self->in_callback = 0;
	self->res_in_callback = 0;
	self->reschedule = 0;
	self->free_on_stop = 0;
	self->stopped = NULL;
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
 * @brief A timer queue.
 *
 * A tree of expiration times. Timers that expire at the same time share one
 * node and are chained in a circular list in the order they were inserted.
 */
typedef struct gp_timer_queue {
	/** @brief A tree of buckets keyed by gp_timer::expires. */
	gp_avl_node *root;
	/** @brief Number of timers in the queue. */
	unsigned int size;
} gp_timer_queue;

/**
 * @brief Returns the timer that expires first.
 *
 * This is the timer gp_timer_queue_process() runs next, i.e. the one to
 * compute a poll timeout from.
 *
 * @param self A timer queue.
 * @return The first timer to expire or NULL if the queue is empty.
 */
gp_timer *gp_timer_queue_first(const gp_timer_queue *self);

/**
 * @brief Prints the queue into stdout, only for debugging.
 *
 * @param self A timer queue.
 */
void gp_timer_queue_dump(const gp_timer_queue *self);

/**
 * @brief Inserts timer into the timer priority queue.
 *
 * Inserts the timer into the queue and sets the expiration time to now +
 * timer->expire.
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
 * @param self A timer queue.
 * @param now A timestamp, usually obtained by calling gp_time_stamp().
 * @param timer A timer to insert.
 */
void gp_timer_queue_ins(gp_timer_queue *self, uint64_t now, gp_timer *timer);

/**
 * @brief Removes timer from timer queue.
 *
 * If called from the timer callback the timer is stopped and the return value
 * from the callback is discarded.
 *
 * @param queue A timer queue.
 * @param timer A timer to remove.
 */
void gp_timer_queue_rem(gp_timer_queue *self, gp_timer *timer);

/**
 * @brief Processes queue, all timers with expires <= now are processed.
 *
 * Timers that return GP_TIMER_STOP from callback are stopped. In the rest of
 * the cases the timer is reinserted into the queue with new expiration time
 * which is now + callback return value.
 *
 * It's possible to reschedule a timer with expires set to 0, which will
 * process the timer on the next process call.
 *
 * @param self A timer queue.
 * @param now A timestamp, usually obtained by calling gp_time_stamp().
 * @return Number of timers processed.
 */
int gp_timer_queue_process(gp_timer_queue *self, uint64_t now);

/**
 * @brief Returns size of the queue, i.e. number of timers.
 *
 * @param self A timer queue.
 * @return A Number of timers in the queue.
 */
static inline unsigned int gp_timer_queue_size(const gp_timer_queue *self)
{
	return self->size;
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
 *
 * @return A newly allocated and initialized timer or NULL on an allocation
 *         failure.
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
