// SPDX-License-Identifier: GPL-2.1-or-later
/*
 * Copyright (C) 2009-2013 Cyril Hrubis <metan@ucw.cz>
 */

/*

  Event Queue.

  This API is internally used by backends to store input events and is not
  supposed to be used directly.

 */

#ifndef INPUT_GP_EVENT_QUEUE_H
#define INPUT_GP_EVENT_QUEUE_H

#include <input/gp_event.h>

struct gp_event_queue {
	/* screen size */
	unsigned int screen_w;
	unsigned int screen_h;

	/* event queue */
	unsigned int queue_first;
	unsigned int queue_last;
	unsigned int queue_size;
	gp_event cur_state;
	gp_event events[GP_EVENT_QUEUE_SIZE];
};

#define GP_EVENT_QUEUE_DECLARE(name, scr_w, scr_h)   \
	gp_event_queue name = {                \
		.screen_w = scr_w,                   \
		.screen_h = scr_h,                   \
		                                     \
		.queue_first = 0,                    \
		.queue_last = 0,                     \
		.queue_size = GP_EVENT_QUEUE_SIZE,   \
		.cur_state = {.cursor_x = scr_w / 2, \
		              .cursor_y = scr_h / 2} \
	}

/*
 * Initializes event queue passed as a pointer. The events array must be
 * queue_size long.
 *
 * If queue_size is set to zero, default value is expected.
 */
void gp_event_queue_init(gp_event_queue *self,
                         unsigned int screen_w, unsigned int screen_h,
                         unsigned int queue_size);

/*
 * Allocates and initializes event queue.
 *
 * If queue_size is set to zero, default value is used.
 */
gp_event_queue *gp_event_queue_alloc(unsigned int screen_w,
                                     unsigned int screen_h,
                                     unsigned int queue_size);

void gp_event_queue_free(gp_event_queue *self);

/*
 * Sets screen (window) size.
 */
void gp_event_queue_set_screen_size(gp_event_queue *self,
                                    unsigned int w, unsigned int h);

/*
 * Sets cursor postion.
 */
void gp_event_queue_set_cursor_pos(gp_event_queue *self,
                                   unsigned int x, unsigned int y);

/*
 * Returns number of events queued in the queue.
 */
unsigned int gp_event_queue_events_queued(gp_event_queue *self);

/*
 * In case there are any events queued, the top event is removed from the
 * queue, copied into the event structure that is passed as argument and
 * non-zero is returned.
 *
 * If there are no events queued the call returns immediately with zero.
 */
int gp_event_queue_get(gp_event_queue *self, gp_event *ev);

/*
 * Same as gp_event_queue_Get but the event is not removed from the queue.
 */
int gp_event_queue_peek(gp_event_queue *self, gp_event *ev);

/*
 * Puts the event in the queue.
 *
 * This is bare call that just copies the event into the queue. Use the calls
 * below instead.
 */
void gp_event_queue_put(gp_event_queue *self, gp_event *ev);

/*
 * Puts event to the top of the queue.
 */
void gp_event_queue_put_back(gp_event_queue *self, gp_event *ev);

struct timeval;

/*
 * Inject event that moves cursor by rx and ry.
 *
 * If timeval is NULL, current time is used.
 */
void gp_event_queue_push_rel(gp_event_queue *self,
                             int32_t rx, int32_t ry, struct timeval *time);

/*
 * Produces relative event that moves cursor to the point x, y.
 *
 * If timeval is NULL, current time is used.
 */
void gp_event_queue_push_rel_to(gp_event_queue *self,
                               uint32_t x, uint32_t y, struct timeval *time);

/*
 * Inject absolute event.
 *
 * If timeval is NULL, current time is used.
 */
void gp_event_queue_push_abs(gp_event_queue *self,
                             uint32_t x, uint32_t y, uint32_t pressure,
                             uint32_t x_max, uint32_t y_max, uint32_t pressure_max,
                             struct timeval *time);

/*
 * Inject event that changes key state (i.e. press, release, repeat).
 *
 * If timeval is NULL, current time is used.
 */
void gp_event_queue_push_key(gp_event_queue *self,
                             uint32_t key, uint8_t code, struct timeval *time);

/*
 * Inject window resize event
 */
void gp_event_queue_push_resize(gp_event_queue *self,
                                uint32_t w, uint32_t h, struct timeval *time);

/*
 * Inject common event.
 */
void gp_event_queue_push(gp_event_queue *self,
                         uint16_t type, uint32_t code, int32_t value,
                         struct timeval *time);

#endif /* INPUT_GP_EVENT_QUEUE_H */
