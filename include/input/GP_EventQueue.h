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


unsigned int GP_EventQueueEventsQueued(struct GP_EventQueue *self);

unsigned int GP_EventQueueGetEvent(struct GP_EventQueue *self,
                                   struct GP_Event *ev);

void GP_EventQueuePutEvent(struct GP_EventQueue *self, struct GP_Event *ev);

#endif /* INPUT_GP_EVENT_QUEUE_H */
