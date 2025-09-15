// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2009-2025 Cyril Hrubis <metan@ucw.cz>
 */

#include <inttypes.h>
#include <poll.h>

#include "core/gp_common.h"
#include <core/gp_transform.h>
#include "core/gp_pixmap.h"
#include <core/gp_debug.h>

#include <input/gp_ev_queue.h>
#include <input/gp_time_stamp.h>

#include <backends/gp_backend.h>
#include <backends/gp_clipboard.h>
#include <backends/gp_backend_input.h>

void gp_backend_update_rect_xyxy(gp_backend *self,
                                 gp_coord x0, gp_coord y0,
                                 gp_coord x1, gp_coord y1)
{
	if (!self->update_rect)
		return;

	GP_TRANSFORM_POINT(self->pixmap, x0, y0);
	GP_TRANSFORM_POINT(self->pixmap, x1, y1);

	if (x1 < x0)
		GP_SWAP(x0, x1);

	if (y1 < y0)
		GP_SWAP(y0, y1);

	if (x0 < 0) {
		if (x1 < 0) {
			GP_WARN("Both x0 and x1 are negative, skipping update");
			return;
		}
		GP_WARN("Negative x0 coordinate %i, clipping to 0", x0);
		x0 = 0;
	}

	if (y0 < 0) {
		if (y1 < 0) {
			GP_WARN("Both y0 and y1 are negative, skipping update");
			return;
		}
		GP_WARN("Negative y0 coordinate %i, clipping to 0", y0);
		y0 = 0;
	}

	gp_coord w = self->pixmap->w;

	if (x1 >= w) {
		if (x0 >= w) {
			GP_WARN("Both x0 and x1 are >= w, skipping update");
			return;
		}
		GP_WARN("Too large x1 coordinate %i, clipping to %u", x1, w - 1);
		x1 = w - 1;
	}

	gp_coord h = self->pixmap->h;

	if (y1 >= h) {
		if (y0 >= h) {
			GP_WARN("Both y0 and y1 are >= h, skipping update");
			return;
		}
		GP_WARN("Too large y1 coordinate %i, clipping to %u", y1, h - 1);
		y1 = h - 1;
	}

	self->update_rect(self, x0, y0, x1, y1);
}

int gp_backend_resize(gp_backend *self, uint32_t w, uint32_t h)
{
	if (!self->set_attr)
		return 1;

	if (w == 0)
		w = self->pixmap->w;

	if (h == 0)
		h = self->pixmap->h;

	if (self->pixmap->w == w && self->pixmap->h == h)
		return 0;

	int vals[] = {w, h};

	return self->set_attr(self, GP_BACKEND_ATTR_SIZE, vals);
}

int gp_backend_resize_ack(gp_backend *self)
{
	GP_DEBUG(2, "Calling backend %s resize_ack()", self->name);

	if (self->resize_ack)
		return self->resize_ack(self);

	return 0;
}

static uint32_t pushevent_callback(gp_timer *self)
{
	gp_event ev;

	ev.type = GP_EV_TMR;
	ev.time = gp_time_stamp();
	ev.tmr = self;

	gp_ev_queue_put(self->_priv, &ev);

	return self->period;
}

void gp_backend_timer_start(gp_backend *self, gp_timer *timer)
{
	if (!timer->callback) {
		timer->callback = pushevent_callback;
		timer->_priv = self->event_queue;
	}

	gp_timer_queue_ins(&self->timers, gp_time_stamp(), timer);
}

void gp_backend_timer_reschedule(gp_backend *self, gp_timer *timer, uint32_t expires_ms)
{
	if (gp_timer_is_running(timer))
		gp_timer_queue_rem(&self->timers, timer);

	timer->expires = expires_ms;

	gp_timer_queue_ins(&self->timers, gp_time_stamp(), timer);
}

void gp_backend_timer_stop(gp_backend *self, gp_timer *timer)
{
	gp_timer_queue_rem(&self->timers, timer);
}

int gp_backend_timer_timeout(gp_backend *self)
{
	uint64_t now;

	if (!self->timers)
		return -1;

	now = gp_time_stamp();

	if (now > self->timers->expires)
		return 0;

	return self->timers->expires - now;
}

static uint32_t backend_task_dispatch(gp_timer *self)
{
	gp_backend *backend = self->priv;

	gp_task_queue_process(backend->task_queue);

	if (gp_task_queue_tasks(backend->task_queue))
		return 0;

	return GP_TIMER_STOP;
}

static gp_timer backend_task_timer = {
	.id = "backend task dispatch",
	.callback = backend_task_dispatch,
};

static void start_task_timer(gp_backend *self)
{
	backend_task_timer.priv = self;
	backend_task_timer.expires = 0;
	gp_backend_timer_start(self, &backend_task_timer);
}

void gp_backend_task_queue_set(gp_backend *self, gp_task_queue *task_queue)
{
	size_t task_cnt = gp_task_queue_tasks(task_queue);

	self->task_queue = task_queue;

	if (!task_cnt)
		return;

	start_task_timer(self);
}

void gp_backend_task_ins(gp_backend *self, gp_task *task)
{
	size_t task_cnt = gp_task_queue_tasks(self->task_queue);

	gp_task_queue_ins(self->task_queue, task);

	if (task_cnt)
		return;

	start_task_timer(self);
}

void gp_backend_task_rem(gp_backend *self, gp_task *task)
{
	gp_task_queue_rem(self->task_queue, task);

	if (!gp_task_queue_tasks(self->task_queue))
		gp_backend_timer_stop(self, &backend_task_timer);
}

static void backend_backlight_keys(gp_backend *self, int up)
{
	int brightness;

	if (up)
		brightness = gp_backend_backlight_inc(self);
	else
		brightness = gp_backend_backlight_dec(self);

	if (brightness == GP_BACKEND_NOTSUPP)
		return;

	gp_ev_queue_push(self->event_queue, GP_EV_SYS, GP_EV_SYS_BACKLIGHT, brightness, 0);
}

gp_event *gp_backend_ev_get(gp_backend *self)
{
	gp_event *ev = gp_ev_queue_get(self->event_queue);

	if (ev) {
		switch (ev->type) {
		case GP_EV_KEY:
			switch (ev->key.key) {
			case GP_KEY_BRIGHTNESS_DOWN:
				if (ev->val)
					backend_backlight_keys(self, 0);
			break;
			case GP_KEY_BRIGHTNESS_UP:
				if (ev->val)
					backend_backlight_keys(self, 1);
			break;
			}
		break;
		}
	}

	return ev;
}

void gp_backend_poll(gp_backend *self)
{
	if (self->timers) {
		if (gp_timer_queue_process(&self->timers, gp_time_stamp()))
			return;
	}

	if (self->poll)
		self->poll(self);

	if (gp_poll_fds(&self->fds))
		gp_poll_wait(&self->fds, 0);
}

static void wait_timers_fds(gp_backend *self, uint64_t now)
{
	int timeout = self->timers->expires - now;

	gp_poll_wait(&self->fds, timeout);
}

/*
 * Polling for backends that does not expose FD to wait on (namely SDL).
 */
static void wait_timers_poll(gp_backend *self)
{
	for (;;) {
		uint64_t now = gp_time_stamp();

		if (gp_timer_queue_process(&self->timers, now))
			return;

		self->poll(self);

		if (gp_poll_fds(&self->fds))
			gp_poll_wait(&self->fds, 0);

		if (gp_backend_ev_queued(self))
			return;

		usleep(10000);
	}
}

void gp_backend_wait(gp_backend *self)
{
	if (gp_backend_ev_queued(self))
		return;

	if (self->timers) {
		uint64_t now = gp_time_stamp();

		/* Get rid of possibly expired timers */
		if (gp_timer_queue_process(&self->timers, now))
			return;

		/* Wait for events or timer expiration */
		if (self->poll)
			wait_timers_poll(self);
		else
			wait_timers_fds(self, now);

		return;
	}

	if (self->wait)
		self->wait(self);
	else
		gp_poll_wait(&self->fds, -1);
}

gp_event *gp_backend_ev_wait(gp_backend *self)
{
	gp_event *ev;

	for (;;) {
		if ((ev = gp_backend_ev_get(self)))
			return ev;

		gp_backend_wait(self);
	}
}

gp_event *gp_backend_ev_poll(gp_backend *self)
{
	gp_event *ev;

	if ((ev = gp_backend_ev_get(self)))
		return ev;

	gp_backend_poll(self);

	if ((ev = gp_backend_ev_get(self)))
		return ev;

	return NULL;
}

void gp_backend_exit(gp_backend *self)
{
	struct gp_clipboard op = {.op = GP_CLIPBOARD_CLEAR};

	if (!self)
		return;

	gp_backend_clipboard(self, &op);

	gp_backend_input_destroy(self);

	self->exit(self);
}
