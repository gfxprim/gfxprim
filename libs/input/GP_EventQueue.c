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

#include "core/GP_Common.h"
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
	self->queue_size = queue_size ? queue_size : GP_EVENT_QUEUE_SIZE;
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

int GP_EventQueueGet(struct GP_EventQueue *self, struct GP_Event *ev)
{
	if (self->queue_first == self->queue_last)
		return 0;

	*ev = self->events[self->queue_first];

	self->queue_first = (self->queue_first + 1) % self->queue_size;

	return 1;
}

static void event_put(struct GP_EventQueue *self, struct GP_Event *ev)
{
	unsigned int next = (self->queue_last + 1) % self->queue_size;

	if (next == self->queue_first) {
		GP_WARN("Event queue full, dropping event.");
		return;
	}
	
	self->events[self->queue_last] = *ev;
	self->queue_last = next;
}

void GP_EventQueuePut(struct GP_EventQueue *self, struct GP_Event *ev)
{
	event_put(self, ev);
}

static void set_time(struct GP_EventQueue *self, struct timeval *time)
{
	if (time == NULL)
		gettimeofday(&self->cur_state.time, NULL);
	else
		self->cur_state.time = *time;
}

static uint32_t clip_rel(uint32_t val, uint32_t max, int32_t rel)
{
	if (rel < 0) {
		if (val < GP_ABS(rel))
			return 0;
		else
			return val + rel;
	}

	if (val + rel >= max)
		return max - 1;

	return val + rel;
}

void GP_EventQueuePushRel(struct GP_EventQueue *self,
                          int32_t rx, int32_t ry, struct timeval *time)
{
	/* event header */
	self->cur_state.type = GP_EV_REL;
	self->cur_state.code = GP_EV_REL_POS;

	self->cur_state.val.rel.rx = rx;
	self->cur_state.val.rel.ry = ry;

 	set_time(self, time);

	/* move the global cursor */
	self->cur_state.cursor_x = clip_rel(self->cur_state.cursor_x, self->screen_w, rx); 
	self->cur_state.cursor_y = clip_rel(self->cur_state.cursor_y, self->screen_h, ry); 

	/* put it into queue */
	event_put(self, &self->cur_state);
}

void GP_EventQueuePushRelTo(struct GP_EventQueue *self,
                            uint32_t x, uint32_t y, struct timeval *time)
{
	if (x > self->screen_w || y > self->screen_h) {
		GP_WARN("x > screen_w or y > screen_h");
		return;
	}
	
	int32_t rx = x - self->cur_state.cursor_x;
	int32_t ry = y - self->cur_state.cursor_y;

	GP_EventQueuePushRel(self, rx, ry, time);
}

void GP_EventQueuePushAbs(struct GP_EventQueue *self,
                          uint32_t x, uint32_t y, uint32_t pressure,
                          uint32_t x_max, uint32_t y_max, uint32_t pressure_max,
                          struct timeval *time)
{
	/* event header */
	self->cur_state.type = GP_EV_ABS;
	self->cur_state.code = GP_EV_ABS_POS;
	self->cur_state.val.abs.x = x; 
	self->cur_state.val.abs.y = y;
	self->cur_state.val.abs.pressure = pressure;
	self->cur_state.val.abs.x_max = x_max; 
	self->cur_state.val.abs.y_max = y_max;
	self->cur_state.val.abs.pressure_max = pressure_max;
	
	set_time(self, time);

	/* 
	 * Set global cursor, the packet could be partial, eg. update only x or
	 * only y. In such case x_max or y_max is zero.
	 */
	if (x_max != 0)	
		self->cur_state.cursor_x = x * (self->screen_w - 1) / x_max;
	
	if (y_max != 0)
		self->cur_state.cursor_y = y * (self->screen_h - 1) / y_max;

	/* put it into queue */
	event_put(self, &self->cur_state);
}

static char keys_to_ascii[] = {
	   0x00, 0x1b,  '1',  '2',  '3',  '4',  '5',  '6',  '7',  '8',
	    '9',  '0',  '-',  '=', 0x08, '\t',  'q',  'w',  'e',  'r',
	    't',  'y',  'u',  'i',  'o',  'p',  '[',  ']', '\n', 0x00,
	    'a',  's',  'd',  'f',  'g',  'h',  'j',  'k',  'l',  ';',
	   '\'',  '`', 0x00, '\\',  'z',  'x',  'c',  'v',  'b',  'n',
	    'm',  ',',  '.',  '/', 0x00,  '*', 0x00,  ' ', 0x00, 0x00,
	   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	   0x00,  '7',  '8',  '9',  '-',  '4',  '5',  '6',  '+',  '1',
	    '2',  '3',  '0',  '.'
};

static char keys_to_ascii_shift[] = {
	   0x00, 0x1b,  '!',  '@',  '#',  '$',  '%',  '^',  '&',  '*',
	    '(',  ')',  '_',  '+', 0x08, '\t',  'Q',  'W',  'E',  'R',
	    'T',  'Y',  'U',  'I',  'O',  'P',  '{',  '}', '\n', 0x00,
	    'A',  'S',  'D',  'F',  'G',  'H',  'J',  'K',  'L',  ':',
	    '"',  '~', 0x00,  '|',  'Z',  'X',  'C',  'V',  'B',  'N',
	    'M',  '<',  '>',  '?', 0x00,  '*', 0x00,  ' ', 0x00, 0x00,
	   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	   0x00,  '7',  '8',  '9',  '-',  '4',  '5',  '6',  '+',  '1',
	    '2',  '3',  '0',  '.'
};

/*
 * Fills key ascii value accordingly to keys pressed.
 */
static void key_to_ascii(struct GP_Event *ev)
{
	unsigned int key = ev->val.key.key;

	ev->val.key.ascii = 0;
	
	if (GP_EventGetKey(ev, GP_KEY_LEFT_SHIFT) ||
	    GP_EventGetKey(ev, GP_KEY_RIGHT_SHIFT)) {
		if (ev->val.key.key < sizeof(keys_to_ascii_shift))
			ev->val.key.ascii = keys_to_ascii_shift[key];
	} else {
		if (ev->val.key.key < sizeof(keys_to_ascii))
			ev->val.key.ascii = keys_to_ascii[key];
	}
}

void GP_EventQueuePushKey(struct GP_EventQueue *self,
                          uint32_t key, uint8_t code, struct timeval *time)
{
	switch (code) {
	case GP_EV_KEY_UP:
		GP_EventResetKey(&self->cur_state, key);	
	break;
	case GP_EV_KEY_DOWN:
		GP_EventSetKey(&self->cur_state, key);	
	break;
	case GP_EV_KEY_REPEAT:
	break;
	default:
		GP_WARN("Invalid key event code %u", code);
		return;
	}

	/* event header */
	self->cur_state.type = GP_EV_KEY;
	self->cur_state.code = code;
	self->cur_state.val.key.key = key;

	key_to_ascii(&self->cur_state);
	
	set_time(self, time);

	/* put it into queue  */
	event_put(self, &self->cur_state);
}

void GP_EventQueuePushResize(struct GP_EventQueue *self,
                             uint32_t w, uint32_t h, struct timeval *time)
{
	/* event header */
	self->cur_state.type = GP_EV_SYS;
	self->cur_state.code = GP_EV_SYS_RESIZE;

	self->cur_state.val.sys.w = w;
	self->cur_state.val.sys.h = h;

 	set_time(self, time);

	/* put it into queue */
	event_put(self, &self->cur_state);
}

void GP_EventQueuePush(struct GP_EventQueue *self,
                       uint16_t type, uint32_t code, int32_t value,
                       struct timeval *time)
{
	switch (type) {
	case GP_EV_KEY:
		GP_EventQueuePushKey(self, code, value, time);
	break;
	default:
		self->cur_state.type = type;
		self->cur_state.code = code;
		self->cur_state.val.val = value;

		set_time(self, time);

		event_put(self, &self->cur_state);
	}
}
