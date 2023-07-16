// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2009-2021 Cyril Hrubis <metan@ucw.cz>
 */

/*

  Gfxprim event layer.

  Events are lowlevel interface to input devices (human interface).

  - Events are notifications that something has changed, eg. button pressed

 */

#ifndef INPUT_GP_EVENT_H
#define INPUT_GP_EVENT_H

#include <stdint.h>
#include <string.h>

#include <input/gp_types.h>
#include <input/gp_keys.h>
#include <input/gp_event_keys.h>

enum gp_event_type {
	GP_EV_KEY = 1, /* key/button press event */
	GP_EV_REL = 2, /* relative event */
	GP_EV_ABS = 3, /* absolute event */
	GP_EV_SYS = 4, /* system events window close, resize... */
	GP_EV_TMR = 5, /* timer expired event */
	GP_EV_UTF = 6, /* unicode character typed on a keyboard */
	GP_EV_MAX = 6, /* maximum, greater values are free */
};

enum gp_event_key_code {
	GP_EV_KEY_UP     = 0,
	GP_EV_KEY_DOWN   = 1,
	GP_EV_KEY_REPEAT = 2,
};

enum gp_event_rel_code {
	GP_EV_REL_POS   = 0,
	GP_EV_REL_WHEEL = 8,
};

enum gp_event_abs_code {
	GP_EV_ABS_POS = 0,
};

enum gp_event_sys_code {
	GP_EV_SYS_QUIT = 0,
	GP_EV_SYS_RESIZE = 1,
	/* clipboard request is ready */
	GP_EV_SYS_CLIPBOARD = 2,
};

struct gp_ev_pos_rel {
	int32_t rx;
	int32_t ry;
};

struct gp_ev_pos_abs {
	uint32_t x, x_max; /* the x is between 0 and x_max */
	uint32_t y, y_max;
	uint32_t pressure, pressure_max;
};

struct gp_ev_key {
	uint32_t key;
};

struct gp_ev_sys {
	uint32_t w, h;
};

struct gp_ev_utf {
	uint32_t ch;
};

#define GP_EVENT_KEY_BITMAP_BYTES 56

struct gp_events_state {
	/** Bitmap of pressed keys including mouse buttons. */
	uint8_t keys_pressed[GP_EVENT_KEY_BITMAP_BYTES];

	/** Cursor coordinates */
	uint32_t cursor_x;
	uint32_t cursor_y;
};

struct gp_event {
	/** enum gp_event_type  */
	uint16_t type;
	/** enum gp_event_*_code */
	uint16_t code;
	union {
		/* generic one integer value */
		int32_t val;
		/* key */
		struct gp_ev_key key;
		/* position */
		struct gp_ev_pos_rel rel;
		struct gp_ev_pos_abs abs;
		/* system event */
		struct gp_ev_sys sys;
		struct gp_ev_utf utf;
		/* timer event */
		gp_timer *tmr;
	};

	/* event timestamp */
	uint64_t time;

	/** Pointer to overall state, pressed keys, cursor position etc. */
	struct gp_events_state *st;
};

/**
 * Returns true if ch is a control character (non-printable)
 */
static inline int gp_ev_utf_is_ctrl(gp_event *ev)
{
	return ev->utf.ch < 0x20 && ev->utf.ch != 0x7f;
}

/*
 * Helpers for setting/getting key bits.
 */
static inline void gp_events_state_press(gp_events_state *self, uint32_t key)
{
	if (key >= GP_EVENT_KEY_BITMAP_BYTES * 8)
		return;

	self->keys_pressed[(key)/8] |= 1<<((key)%8);
}

static inline int gp_events_state_pressed(gp_events_state *self, uint32_t key)
{
	if (key >= GP_EVENT_KEY_BITMAP_BYTES * 8)
		return 0;

	return !!(self->keys_pressed[(key)/8] & (1<<((key)%8)));
}

static inline void gp_events_state_release(gp_events_state *self, uint32_t key)
{
	if (key >= GP_EVENT_KEY_BITMAP_BYTES * 8)
		return;

	self->keys_pressed[(key)/8] &= ~(1<<((key)%8));
}

static inline void gp_events_state_release_all(gp_events_state *self)
{
	memset(self->keys_pressed, 0, sizeof(self->keys_pressed));
}

static inline int gp_ev_key_pressed(gp_event *ev, uint32_t key)
{
	if (!ev->st)
		return 0;

	return gp_events_state_pressed(ev->st, key);
}

#define gp_ev_any_key_pressed(ev, ...) gp_ev_any_key_pressed_(ev, __VA_ARGS__, 0)
#define gp_ev_all_keys_pressed(ev, ...) gp_ev_all_keys_pressed_(ev, __VA_ARGS__, 0)

/**
 * Returns non-zero if any key from a list is pressed.
 */
int gp_ev_any_key_pressed_(gp_event *ev, ...);

/**
 * Returns non-zero if all keys from a list are pressed.
 */
int gp_ev_all_keys_pressed_(gp_event *ev, ...);

/*
 * Dumps event into stdout.
 */
void gp_ev_dump(gp_event *ev);

#endif /* INPUT_GP_EVENT_H */
