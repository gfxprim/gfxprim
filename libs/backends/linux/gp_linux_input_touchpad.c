// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2009-2025 Cyril Hrubis <metan@ucw.cz>
 */

#include <limits.h>

#include <core/gp_debug.h>
#include <core/gp_common.h>
#include <core/gp_clamp.h>

#include "gp_linux_input_dev.h"

#define INVALID_LAST_COORD INT_MIN

static void input_abs_touchpad(struct linux_input *self, struct input_event *ev)
{
	switch (ev->code) {
	case ABS_X:
		self->tp.last_x = self->tp.abs_x;
		self->tp.abs_x = ev->value;
		GP_DEBUG(4, "ABS X %i", ev->value);
	break;
	case ABS_Y:
		self->tp.last_y = self->tp.abs_y;
		self->tp.abs_y = ev->value;
		GP_DEBUG(4, "ABS Y %i", ev->value);
	break;
	default:
		GP_DEBUG(3, "ABS unhandled 0x%02x code", ev->code);
	}
}

static void input_key_touchpad(struct linux_input *self, struct input_event *ev)
{
	GP_DEBUG(4, "Touchpad key %s event", gp_ev_key_name(ev->code));

	switch (ev->code) {
	case GP_BTN_TOUCH:
	break;
	case GP_BTN_TOOL_FINGER:
		self->tp.abs_x = INVALID_LAST_COORD;
		self->tp.abs_y = INVALID_LAST_COORD;
		self->tp.last_x = INVALID_LAST_COORD;
		self->tp.last_y = INVALID_LAST_COORD;

		self->tp.single_tap = ev->value;
		GP_DEBUG(4, "Single tap");
	break;
	case GP_BTN_TOOL_DOUBLE_TAP:
		self->tp.double_tap = ev->value;
		GP_DEBUG(4, "Double tap");
	break;
	case GP_BTN_TOOL_TRIPLE_TAP:
		self->tp.triple_tap = ev->value;
		GP_DEBUG(4, "Triple tap");
	break;
	default:
		gp_ev_queue_push_key(self->backend->event_queue, ev->code, ev->value, 0);
	}
}

static void input_sync_touchpad(struct linux_input *self)
{
	gp_ev_queue *ev_queue = self->backend->event_queue;

	if (self->tp.triple_tap)
		return;

	if (self->tp.double_tap) {
		int rel_x = 0;

		if (self->tp.last_x != INVALID_LAST_COORD)
			rel_x = self->tp.last_x - self->tp.abs_x;

		if (rel_x) {
			GP_DEBUG(4, "Relative wheel %i", rel_x);
			gp_ev_queue_push_wheel(ev_queue, rel_x, 0);
		}
		return;
	}

	if (self->tp.single_tap) {
		int rel_x = 0;
		int rel_y = 0;

		if (self->tp.last_x != INVALID_LAST_COORD)
			rel_x = self->tp.abs_x - self->tp.last_x;

		if (self->tp.last_y != INVALID_LAST_COORD)
			rel_y = self->tp.abs_y - self->tp.last_y;

		if (rel_x || rel_y) {
			GP_DEBUG(4, "Relative movement %i %i", rel_x, rel_y);
			gp_ev_queue_push_rel(ev_queue, rel_x, rel_y, 0);
		}
	}
}

__attribute__((visibility ("hidden")))
void input_touchpad(struct linux_input *self, struct input_event *ev)
{

	switch (ev->type) {
	case EV_REL:
		GP_WARN("Relative events for touchpad!?");
	break;
	case EV_ABS:
		input_abs_touchpad(self, ev);
	break;
	case EV_KEY:
		input_key_touchpad(self, ev);
	break;
	case EV_SYN:
		input_sync_touchpad(self);
	break;
	default:
		GP_DEBUG(3, "Unhandled type 0x%02x", ev->type);
	}
}
