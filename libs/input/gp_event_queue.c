// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2009-2021 Cyril Hrubis <metan@ucw.cz>
 */

#include <string.h>

#include <core/gp_common.h>
#include <core/gp_debug.h>

#include <input/gp_time_stamp.h>
#include <input/gp_keymap.h>
#include <input/gp_event_queue.h>

void gp_event_queue_init(gp_event_queue *self,
                         unsigned int screen_w, unsigned int screen_h,
                         unsigned int queue_size, int flags)
{
	self->screen_w = screen_w;
	self->screen_h = screen_h;

	memset(&self->state, 0, sizeof(self->state));

	self->state.cursor_x = screen_w / 2;
	self->state.cursor_y = screen_h / 2;

	self->keymap = NULL;

	self->queue_first = 0;
	self->queue_last = 0;
	self->queue_size = queue_size ? queue_size : GP_EVENT_QUEUE_SIZE;

	if (flags & GP_EVENT_QUEUE_LOAD_KEYMAP)
		self->keymap = gp_keymap_load(NULL);
}

void gp_event_queue_set_screen_size(gp_event_queue *self,
                                    unsigned int w, unsigned int h)
{
	GP_DEBUG(1, "Resizing input queue screen to %ux%u", w, h);

	self->screen_w = w;
	self->screen_h = h;

	/* clip cursor */
	if (self->state.cursor_x >= w)
		self->state.cursor_x = w - 1;

	if (self->state.cursor_y >= h)
		self->state.cursor_y = h - 1;
}

void gp_event_queue_set_cursor_pos(gp_event_queue *self,
                                   unsigned int x, unsigned int y)
{
	if (x >= self->screen_w || y >= self->screen_h) {
		GP_WARN("Attempt to set cursor %u,%u out of the screen %ux%u",
		        x, y, self->screen_w, self->screen_h);
		return;
	}

	self->state.cursor_x = x;
	self->state.cursor_y = y;
}

unsigned int gp_event_queue_events(gp_event_queue *self)
{
	if (self->queue_first <= self->queue_last)
		return self->queue_last - self->queue_first;

	return self->queue_size - (self->queue_first - self->queue_last);
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

gp_event *gp_event_queue_get(gp_event_queue *self)
{
	if (self->queue_first == self->queue_last)
		return NULL;

	gp_event *ev = &self->events[self->queue_first];

	self->queue_first = (self->queue_first + 1) % self->queue_size;

	switch (ev->type) {
	case GP_EV_KEY:
		/* Update bitmap of pressed keys */
		switch (ev->code) {
		case GP_EV_KEY_UP:
			gp_events_state_release(&self->state, ev->val);
		break;
		case GP_EV_KEY_DOWN:
			gp_events_state_press(&self->state, ev->val);
		break;
		}

		// TODO: Add queue keymap
		//key_to_ascii(self, ev);
	break;
	case GP_EV_REL:
		/* Move the global cursor */
		if (ev->code == GP_EV_REL_POS) {
			self->state.cursor_x = clip_rel(self->state.cursor_x,
					                self->screen_w, ev->rel.rx);
			self->state.cursor_y = clip_rel(self->state.cursor_y,
				                        self->screen_h, ev->rel.ry);
		}
	break;
	case GP_EV_ABS:
		/* Move the global cursor */
		if (ev->code == GP_EV_ABS_POS) {
			/*
			 * Set global cursor, the packet could be partial, eg. update only x or
			 * only y. In such case x_max or y_max is zero.
			 */
			if (ev->abs.x_max != 0)
				self->state.cursor_x = ev->abs.x * (self->screen_w - 1) / ev->abs.x_max;

			if (ev->abs.y_max != 0)
				self->state.cursor_y = ev->abs.y * (self->screen_h - 1) / ev->abs.y_max;
		}
	break;
	}

	ev->st = &self->state;

	return ev;
}

gp_event *gp_event_queue_peek(gp_event_queue *self)
{
	if (self->queue_first == self->queue_last)
		return NULL;

	return &(self->events[self->queue_first]);
}

/*
 * We avoid overriding an position before the queue_first since that may have
 * been returned by gp_event_get() previously.
 */
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

static void set_time(gp_event *ev, uint64_t time)
{
	if (!time)
		ev->time = gp_time_stamp();
	else
		ev->time = time;
}

void gp_event_queue_push_rel(gp_event_queue *self,
                             int32_t rx, int32_t ry, uint64_t time)
{
	gp_event ev = {
		.type = GP_EV_REL,
		.code = GP_EV_REL_POS,
		.rel = {.rx = rx, ry = ry},
	};

	set_time(&ev, time);

	event_put(self, &ev);
}

void gp_event_queue_push_rel_to(gp_event_queue *self,
                               uint32_t x, uint32_t y, uint64_t time)
{
	if (x > self->screen_w || y > self->screen_h) {
		GP_WARN("x > screen_w or y > screen_h");
		return;
	}

	int32_t rx = x - self->state.cursor_x;
	int32_t ry = y - self->state.cursor_y;

	gp_event_queue_push_rel(self, rx, ry, time);
}

void gp_event_queue_push_abs(gp_event_queue *self,
                             uint32_t x, uint32_t y, uint32_t pressure,
                             uint32_t x_max, uint32_t y_max, uint32_t pressure_max,
                             uint64_t time)
{
	gp_event ev = {
		.type = GP_EV_ABS,
		.code = GP_EV_ABS_POS,
		.abs = {
			.x = x,
			.y = y,
			.pressure = pressure,
			.x_max = x_max,
			.y_max = y_max,
			.pressure_max = pressure_max,
		}
	};

	set_time(&ev, time);

	event_put(self, &ev);
}

void gp_event_queue_push_key(gp_event_queue *self,
                             uint32_t key, uint8_t code,
			     uint64_t time)
{
	switch (code) {
	case GP_EV_KEY_UP:
	break;
	case GP_EV_KEY_DOWN:
	break;
	case GP_EV_KEY_REPEAT:
	break;
	default:
		GP_WARN("Invalid key event code %u", code);
		return;
	}

	gp_event ev = {
		.type = GP_EV_KEY,
		.code = code,
		.key = {.key = key}
	};

	set_time(&ev, time);

	int dead_key = 0;

	if (self->keymap)
		dead_key = gp_keymap_event_key(self->keymap, self, &ev);

	if (!dead_key)
		event_put(self, &ev);
}

void gp_event_queue_push_utf(gp_event_queue *self, uint32_t utf_ch,
                             uint64_t time)
{
	gp_event ev = {
		.type = GP_EV_UTF,
		.utf = {.ch = utf_ch}
	};

	set_time(&ev, time);

	event_put(self, &ev);
}

void gp_event_queue_push_resize(gp_event_queue *self,
                                uint32_t w, uint32_t h, uint64_t time)
{
	gp_event ev = {
		.type = GP_EV_SYS,
		.code = GP_EV_SYS_RESIZE,
		.sys = {.w = w, .h = h},
	};

	set_time(&ev, time);

	event_put(self, &ev);
}

void gp_event_queue_push(gp_event_queue *self,
                         uint16_t type, uint32_t code, int32_t value,
                         uint64_t time)
{
	if (type == GP_EV_KEY) {
		gp_event_queue_push_key(self, code, value, time);
		return;
	}

	gp_event ev = {
		.type = type,
		.code = code,
		.val = value,
	};

	set_time(&ev, time);

	event_put(self, &ev);
}
