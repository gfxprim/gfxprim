// SPDX-License-Identifier: LGPL-2.1-or-later
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

#define GP_EVENT_KEY_BITMAP_BYTES 36

struct gp_event_key_state {
	/** Bitmap of pressed keys including mouse buttons. */
	uint8_t keys_pressed[GP_EVENT_KEY_BITMAP_BYTES];
};

/*
 * Helpers for setting/getting key bits.
 */
static inline void gp_event_key_state_press(gp_event_key_state *self, uint32_t key)
{
	if (key >= GP_EVENT_KEY_BITMAP_BYTES * 8)
		return;

	self->keys_pressed[(key)/8] |= 1<<((key)%8);
}

static inline int gp_event_key_state_pressed(gp_event_key_state *self, uint32_t key)
{
	if (key >= GP_EVENT_KEY_BITMAP_BYTES * 8)
		return 0;

	return !!(self->keys_pressed[(key)/8] & (1<<((key)%8)));
}

static inline void gp_event_key_state_release(gp_event_key_state *self, uint32_t key)
{
	if (key >= GP_EVENT_KEY_BITMAP_BYTES * 8)
		return;

	self->keys_pressed[(key)/8] &= ~(1<<((key)%8));
}

#define GP_EVENT_QUEUE_SIZE 32

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

	/*
	 * Accumulated keyboard state, valid only for event removed by the last
	 * gp_event_queue_get().
	 */
	gp_event_key_state state;
};

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
unsigned int gp_event_queue_events(gp_event_queue *self);

/*
 * In case there are any events queued a pointer to a top event in the queue is
 * returned. The pointer is valid until next call to gp_event_queue_get().
 *
 * The pointer is also invalidated by a call to gp_event_queue_put_back().
 *
 * If there are no events queued the call returns NULL.
 */
gp_event *gp_event_queue_get(gp_event_queue *self);

/*
 * Same as gp_event_queue_get() but the event is not removed from the queue.
 * The pointer is valid until a call to gp_event_queue_get().
 *
 * If there are no events queued the calll returns NULL.
 *
 * The keyboard state bitmask is _NOT_ modified.
 */
gp_event *gp_event_queue_peek(gp_event_queue *self);

/*
 * Puts the event in the queue.
 *
 * This is bare call that just copies the event into the queue. Use the calls
 * below instead.
 */
void gp_event_queue_put(gp_event_queue *self, gp_event *ev);

/*
 * Puts event to the top of the queue, i.e. this event is going to be returned
 * by a next call to gp_event_queue_get().
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

static inline int gp_event_queue_key_pressed(gp_event_queue *self, uint32_t key)
{
	return gp_event_key_state_pressed(&self->state, key);
}

#endif /* INPUT_GP_EVENT_QUEUE_H */
