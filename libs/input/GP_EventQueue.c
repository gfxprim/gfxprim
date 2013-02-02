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

#include "core/GP_Debug.h"

#include "input/GP_EventQueue.h"

void GP_EventQueueInit(struct GP_EventQueue *self,
                       unsigned int screen_w, unsigned int screen_h,
                       unsigned int queue_size)
{
	self->screen_w = screen_w;
	self->screen_h = screen_h;

	self->cur_state.cursor_x = screen_w / 2;
	self->cur_state.cursor_y = screen_h / 2;

	self->queue_first = 0;
	self->queue_last = 0;
	self->queue_size = queue_size;
}

struct GP_EventQueue *GP_EventQueueAlloc(unsigned int screen_w,
                                         unsigned int screen_h,
                                         unsigned int queue_size)
{
	size_t size;
	struct GP_EventQueue *new;
	
	size = sizeof(struct GP_EventQueue) +
	       (queue_size - GP_EVENT_QUEUE_SIZE) * sizeof(struct GP_Event);
	
	new = malloc(size);

	if (new == NULL) {
		GP_WARN("Malloc failed :(");
		return NULL;
	}

	GP_EventQueueInit(new, screen_w, screen_h, queue_size);

	return new;
}

void GP_EventQueueFree(struct GP_EventQueue *self)
{
	free(self);
}

void GP_EventQueueSetScreenSize(struct GP_EventQueue *self,
                                unsigned int w, unsigned int h)
{
	GP_DEBUG(1, "Resizing input queue screen to %ux%u", w, h);

	self->screen_w = w;
	self->screen_h = h;

	/* clip cursor */
	if (self->cur_state.cursor_x >= w)
		self->cur_state.cursor_x = w - 1;
	
	if (self->cur_state.cursor_y >= h)
		self->cur_state.cursor_y = h - 1;
}

void GP_EventQueueSetCursorPosition(struct GP_EventQueue *self,
                                    unsigned int x, unsigned int y)
{
	if (x >= self->screen_w || y >= self->screen_h) {
		GP_WARN("Attempt to set cursor %u,%u out of the screen %ux%u",
		        x, y, self->screen_w, self->screen_h);
		return;
	}
	
	self->cur_state.cursor_x = x;
	self->cur_state.cursor_y = y;
}

unsigned int GP_EventQueueEventsQueued(struct GP_EventQueue *self)
{
	if (self->queue_first <= self->queue_last)
		return self->queue_last - self->queue_first;

	return self->queue_size - (self->queue_last - self->queue_first);
}

unsigned int GP_EventQueueGetEvent(struct GP_EventQueue *self,
                                   struct GP_Event *ev)
{
	if (self->queue_first == self->queue_last)
		return 0;

	*ev = self->events[self->queue_first];

	self->queue_first = (self->queue_first + 1) % self->queue_size;

	return 1;
}

void GP_EventQueuePutEvent(struct GP_EventQueue *self, struct GP_Event *ev)
{
	unsigned int next = (self->queue_last + 1) % self->queue_size;

	if (next == self->queue_first) {
		GP_WARN("Event queue full, dropping event.");
		return;
	}
	
	self->events[self->queue_last] = *ev;
	self->queue_last = next;
}
