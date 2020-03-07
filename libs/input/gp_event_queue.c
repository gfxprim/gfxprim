// SPDX-License-Identifier: GPL-2.1-or-later
/*
 * Copyright (C) 2009-2013 Cyril Hrubis <metan@ucw.cz>
 */

#include <string.h>

#include "core/gp_common.h"
#include <core/gp_debug.h>

#include <input/gp_event_queue.h>

void gp_event_queue_init(gp_event_queue *self,
                         unsigned int screen_w, unsigned int screen_h,
                         unsigned int queue_size)
{
	self->screen_w = screen_w;
	self->screen_h = screen_h;

	memset(&self->cur_state, 0, sizeof(self->cur_state));

	self->cur_state.cursor_x = screen_w / 2;
	self->cur_state.cursor_y = screen_h / 2;

	self->queue_first = 0;
	self->queue_last = 0;
	self->queue_size = queue_size ? queue_size : GP_EVENT_QUEUE_SIZE;
}

gp_event_queue *gp_event_queue_alloc(unsigned int screen_w,
                                     unsigned int screen_h,
                                     unsigned int queue_size)
{
	size_t size;
	gp_event_queue *new;

	size = sizeof(gp_event_queue) +
	       (queue_size - GP_EVENT_QUEUE_SIZE) * sizeof(gp_event);

	new = malloc(size);

	if (new == NULL) {
		GP_WARN("Malloc failed :(");
		return NULL;
	}

	gp_event_queue_init(new, screen_w, screen_h, queue_size);

	return new;
}

void gp_event_queue_free(gp_event_queue *self)
{
	free(self);
}

void gp_event_queue_set_screen_size(gp_event_queue *self,
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

void gp_event_queue_set_cursor_pos(gp_event_queue *self,
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

unsigned int gp_event_queue_events_queued(gp_event_queue *self)
{
	if (self->queue_first <= self->queue_last)
		return self->queue_last - self->queue_first;

	return self->queue_size - (self->queue_last - self->queue_first);
}

int gp_event_queue_get(gp_event_queue *self, gp_event *ev)
{
	if (self->queue_first == self->queue_last)
		return 0;

	*ev = self->events[self->queue_first];

	self->queue_first = (self->queue_first + 1) % self->queue_size;

	return 1;
}

int gp_event_queue_peek(gp_event_queue *self, gp_event *ev)
{
	if (self->queue_first == self->queue_last)
		return 0;

	*ev = self->events[self->queue_first];

	return 1;
}

static void event_put(gp_event_queue *self, gp_event *ev)
{
	unsigned int next = (self->queue_last + 1) % self->queue_size;

	if (next == self->queue_first) {
		GP_WARN("Event queue full, dropping event.");
		return;
	}

	self->events[self->queue_last] = *ev;
	self->queue_last = next;
}

static void event_put_back(gp_event_queue *self, gp_event *ev)
{
	unsigned int prev;

	if (self->queue_first == 0)
		prev = self->queue_last - 1;
	else
		prev = self->queue_first - 1;

	if (prev == self->queue_last) {
		GP_WARN("Event queue full, dropping event.");
		return;
	}

	self->events[prev] = *ev;
	self->queue_first = prev;
}

void gp_event_queue_put(gp_event_queue *self, gp_event *ev)
{
	event_put(self, ev);
}

void gp_event_queue_put_back(gp_event_queue *self, gp_event *ev)
{
	event_put_back(self, ev);
}

static void set_time(gp_event_queue *self, struct timeval *time)
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

void gp_event_queue_push_rel(gp_event_queue *self,
                             int32_t rx, int32_t ry, struct timeval *time)
{
	/* event header */
	self->cur_state.type = GP_EV_REL;
	self->cur_state.code = GP_EV_REL_POS;

	self->cur_state.val.rel.rx = rx;
	self->cur_state.val.rel.ry = ry;

	set_time(self, time);

	/* move the global cursor */
	self->cur_state.cursor_x = clip_rel(self->cur_state.cursor_x,
	                                    self->screen_w, rx);
	self->cur_state.cursor_y = clip_rel(self->cur_state.cursor_y,
	                                    self->screen_h, ry);

	/* put it into queue */
	event_put(self, &self->cur_state);
}

void gp_event_queue_push_rel_to(gp_event_queue *self,
                               uint32_t x, uint32_t y, struct timeval *time)
{
	if (x > self->screen_w || y > self->screen_h) {
		GP_WARN("x > screen_w or y > screen_h");
		return;
	}

	int32_t rx = x - self->cur_state.cursor_x;
	int32_t ry = y - self->cur_state.cursor_y;

	gp_event_queue_push_rel(self, rx, ry, time);
}

void gp_event_queue_push_abs(gp_event_queue *self,
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
static void key_to_ascii(gp_event *ev)
{
	unsigned int key = ev->val.key.key;

	ev->val.key.ascii = 0;

	if (gp_event_get_key(ev, GP_KEY_LEFT_SHIFT) ||
	    gp_event_get_key(ev, GP_KEY_RIGHT_SHIFT)) {
		if (ev->val.key.key < sizeof(keys_to_ascii_shift))
			ev->val.key.ascii = keys_to_ascii_shift[key];
	} else {
		if (ev->val.key.key < sizeof(keys_to_ascii))
			ev->val.key.ascii = keys_to_ascii[key];
	}
}

void gp_event_queue_push_key(gp_event_queue *self,
                             uint32_t key, uint8_t code, struct timeval *time)
{
	switch (code) {
	case GP_EV_KEY_UP:
		gp_event_reset_key(&self->cur_state, key);
	break;
	case GP_EV_KEY_DOWN:
		gp_event_set_key(&self->cur_state, key);
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

void gp_event_queue_push_resize(gp_event_queue *self,
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

void gp_event_queue_push(gp_event_queue *self,
                         uint16_t type, uint32_t code, int32_t value,
                         struct timeval *time)
{
	switch (type) {
	case GP_EV_KEY:
		gp_event_queue_push_key(self, code, value, time);
	break;
	default:
		self->cur_state.type = type;
		self->cur_state.code = code;
		self->cur_state.val.val = value;

		set_time(self, time);

		event_put(self, &self->cur_state);
	}
}
