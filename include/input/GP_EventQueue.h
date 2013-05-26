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

  Event Queue.

  This API is internally used by backends to store input events and is not
  supposed to be used directly.

 */

#ifndef INPUT_GP_EVENT_QUEUE_H
#define INPUT_GP_EVENT_QUEUE_H

#include "input/GP_Event.h"

typedef struct GP_EventQueue {
	/* screen size */
	unsigned int screen_w;
	unsigned int screen_h;

	/* event queue */
	unsigned int queue_first;
	unsigned int queue_last;
	unsigned int queue_size;
	struct GP_Event cur_state;
	struct GP_Event events[GP_EVENT_QUEUE_SIZE];
} GP_EventQueue;

#define GP_EVENT_QUEUE_DECLARE(name, scr_w, scr_h)   \
	struct GP_EventQueue name = {                \
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
void GP_EventQueueInit(struct GP_EventQueue *self,
                       unsigned int screen_w, unsigned int screen_h,
                       unsigned int queue_size);

/*
 * Allocates and initializes event queue.
 *
 * If queue_size is set to zero, default value is used.
 */
struct GP_EventQueue *GP_EventQueueAlloc(unsigned int screen_w,
                                         unsigned int screen_h,
                                         unsigned int queue_size);

void GP_EventQueueFree(struct GP_EventQueue *self);

/*
 * Sets screen (window) size.
 */
void GP_EventQueueSetScreenSize(struct GP_EventQueue *self,
                                unsigned int w, unsigned int h);

/*
 * Sets cursor postion.
 */
void GP_EventQueueSetCursorPosition(struct GP_EventQueue *self,
                                    unsigned int x, unsigned int y);

/*
 * Returns number of events queued in the queue.
 */
unsigned int GP_EventQueueEventsQueued(struct GP_EventQueue *self);

/*
 * In case there are any events queued, the top event is removed from the
 * queue, copied into the event structure that is passed as argument and
 * non-zero is returned.
 *
 * If there are no events queued the call returns immediately with zero.
 */
int GP_EventQueueGet(struct GP_EventQueue *self, struct GP_Event *ev);

/*
 * Same as GP_EventQueueGet but the event is not removed from the queue.
 */
int GP_EventQueuePeek(struct GP_EventQueue *self, struct GP_Event *ev);

/*
 * Puts the event in the queue.
 *
 * This is bare call that just copies the event into the queue. Use the calls
 * bellow instead.
 */
void GP_EventQueuePut(struct GP_EventQueue *self, struct GP_Event *ev);

/*
 * Puts event to the top of the queue.
 */
void GP_EventQueuePutBack(struct GP_EventQueue *self, struct GP_Event *ev);

struct timeval;

/*
 * Inject event that moves cursor by rx and ry.
 *
 * If timeval is NULL, current time is used.
 */
void GP_EventQueuePushRel(struct GP_EventQueue *self,
                          int32_t rx, int32_t ry, struct timeval *time);

/*
 * Produces relative event that moves cursor to the point x, y.
 *
 * If timeval is NULL, current time is used.
 */
void GP_EventQueuePushRelTo(struct GP_EventQueue *self,
                            uint32_t x, uint32_t y, struct timeval *time);

/*
 * Inject absolute event.
 *
 * If timeval is NULL, current time is used.
 */
void GP_EventQueuePushAbs(struct GP_EventQueue *self,
                          uint32_t x, uint32_t y, uint32_t pressure,
                          uint32_t x_max, uint32_t y_max, uint32_t pressure_max,
                          struct timeval *time);

/*
 * Inject event that changes key state (i.e. press, release, repeat).
 *
 * If timeval is NULL, current time is used.
 */
void GP_EventQueuePushKey(struct GP_EventQueue *self,
                          uint32_t key, uint8_t code, struct timeval *time);

/*
 * Inject window resize event
 */
void GP_EventQueuePushResize(struct GP_EventQueue *self,
                             uint32_t w, uint32_t h, struct timeval *time);

/*
 * Inject common event.
 */
void GP_EventQueuePush(struct GP_EventQueue *self,
                       uint16_t type, uint32_t code, int32_t value,
                       struct timeval *time);

#endif /* INPUT_GP_EVENT_QUEUE_H */
