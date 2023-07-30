// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2009-2021 Cyril Hrubis <metan@ucw.cz>
 */

/*

  Event Queue.

  This API is internally used by backends to store input events and is not
  supposed to be used directly.

 */

#ifndef INPUT_GP_EV_QUEUE_H
#define INPUT_GP_EV_QUEUE_H

#include <input/gp_ev_feedback.h>
#include <input/gp_event.h>

#define GP_EVENT_QUEUE_SIZE 32

struct gp_ev_queue {
	/* screen size */
	unsigned int screen_w;
	unsigned int screen_h;

	/* Cursor position valid for the last event in the queue */
	uint32_t cursor_x;
	uint32_t cursor_y;

	/* event queue */
	unsigned int queue_first;
	unsigned int queue_last;
	unsigned int queue_size;

	/* keymap needed only for framebuffer */
	gp_keymap *keymap;

	/* list of callbacks to turn on/off leds and other feedback */
	gp_ev_feedback *feedbacks_list;

	/*
	 * Accumulated state, pressed keys, cursor position, etc.
	 *
	 * Valid only for event removed by the last gp_ev_queue_get().
	 */
	gp_events_state state;

	gp_event events[GP_EVENT_QUEUE_SIZE];
};

enum gp_ev_queue_flags {
	GP_EVENT_QUEUE_LOAD_KEYMAP = 0x01,
};

/*
 * Initializes event queue passed as a pointer. The events array must be
 * queue_size long.
 *
 * If queue_size is set to zero, default value is expected.
 */
void gp_ev_queue_init(gp_ev_queue *self,
                         unsigned int screen_w, unsigned int screen_h,
                         unsigned int queue_size, int flags);

/*
 * Sets screen (window) size.
 */
void gp_ev_queue_set_screen_size(gp_ev_queue *self,
                                    unsigned int w, unsigned int h);

/**
 * @brief Registers an input device feedback callback.
 *
 * @self An event queue.
 * @feedback An input device feedback callback.
 */
static inline void gp_ev_queue_feedback_register(gp_ev_queue *self, gp_ev_feedback *feedback)
{
	gp_ev_feedback_register(&self->feedbacks_list, feedback);
}

/**
 * @brief Unregisters an input device feedback callback.
 *
 * @self An event queue.
 * @feedback An input device feedback callback.
 */
static inline void gp_ev_queue_feedback_unregister(gp_ev_queue *self, gp_ev_feedback *feedback)
{
	gp_ev_feedback_unregister(&self->feedbacks_list, feedback);
}

/**
 * @brief Calls a all feedback handlers with a specified value.
 *
 * @self And event queue
 * @val A value to be set
 */
static inline void gp_ev_queue_feedback_set_all(gp_ev_queue *self, gp_ev_feedback_op *op)
{
	gp_ev_feedback_set_all(self->feedbacks_list, op);
}

/*
 * Sets cursor postion.
 */
void gp_ev_queue_set_cursor_pos(gp_ev_queue *self,
                                   unsigned int x, unsigned int y);

/*
 * Returns number of events queued in the queue.
 */
unsigned int gp_ev_queue_events(gp_ev_queue *self);

/**
 * @brief Returns true if queue is full.
 *
 * @self An event queue.
 * @return Non-zero if queue is full zero otherwise.
 */
static inline int gp_ev_queue_full(gp_ev_queue *self)
{
	unsigned int next = (self->queue_last + 1) % self->queue_size;

	return next == self->queue_first;
}

/*
 * In case there are any events queued a pointer to a top event in the queue is
 * returned. The pointer is valid until next call to gp_ev_queue_get().
 *
 * The pointer is also invalidated by a call to gp_ev_queue_put_back().
 *
 * If there are no events queued the call returns NULL.
 */
gp_event *gp_ev_queue_get(gp_ev_queue *self);

/*
 * Same as gp_ev_queue_get() but the event is not removed from the queue.
 * The pointer is valid until a call to gp_ev_queue_get().
 *
 * If there are no events queued the calll returns NULL.
 *
 * The keyboard state bitmask is _NOT_ modified.
 */
gp_event *gp_ev_queue_peek(gp_ev_queue *self);

/*
 * Puts the event in the queue.
 *
 * This is bare call that just copies the event into the queue. Use the calls
 * below instead.
 */
void gp_ev_queue_put(gp_ev_queue *self, gp_event *ev);

/*
 * Puts event to the top of the queue, i.e. this event is going to be returned
 * by a next call to gp_ev_queue_get().
 */
void gp_ev_queue_put_back(gp_ev_queue *self, gp_event *ev);

struct timeval;

/*
 * Inject event that moves cursor by rx and ry.
 *
 * If timeval is NULL, current time is used.
 */
void gp_ev_queue_push_rel(gp_ev_queue *self,
                             int32_t rx, int32_t ry, uint64_t time);

/*
 * Produces relative event that moves cursor to the point x, y.
 *
 * If timeval is NULL, current time is used.
 */
void gp_ev_queue_push_rel_to(gp_ev_queue *self,
                               uint32_t x, uint32_t y, uint64_t time);

/*
 * Inject absolute event.
 *
 * If timeval is NULL, current time is used.
 */
void gp_ev_queue_push_abs(gp_ev_queue *self,
                             uint32_t x, uint32_t y, uint32_t pressure,
                             uint32_t x_max, uint32_t y_max, uint32_t pressure_max,
                             uint64_t time);

/*
 * Inject event that changes key state (i.e. press, release, repeat).
 *
 * @key  Physical key pressed on keyboard
 * @code Action press/release/repeat
 * @utf An UTF32 mapping, if available, pass zero if none
 * @timeval A timestamp; if NULL current time is used
 */
void gp_ev_queue_push_key(gp_ev_queue *self,
                             uint32_t key, uint8_t code,
                             uint64_t time);

/**
 * @brief Injects an unicode character typed on keyboard
 *
 * @self An input queue.
 * @utf_ch An unicode character.
 * @timeval A timestamp; if NULL current time is used
 */
void gp_ev_queue_push_utf(gp_ev_queue *self, uint32_t utf_ch,
                             uint64_t time);

/*
 * Inject window resize event
 */
void gp_ev_queue_push_resize(gp_ev_queue *self,
                                uint32_t w, uint32_t h, uint64_t time);

/*
 * Inject common event.
 */
void gp_ev_queue_push(gp_ev_queue *self,
                         uint16_t type, uint32_t code, int32_t value,
                         uint64_t time);

/*
 * Pushes mouse wheel movement.
 */
static inline void gp_ev_queue_push_wheel(gp_ev_queue *self,
                                             int32_t val, uint64_t time)
{
	gp_ev_queue_push(self, GP_EV_REL, GP_EV_REL_WHEEL, val, time);
}

#endif /* INPUT_GP_EV_QUEUE_H */
