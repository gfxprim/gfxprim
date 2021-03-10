// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2009-2013 Cyril Hrubis <metan@ucw.cz>
 */

/*

  Gfxprim event layer.

  Events are lowlevel interface to input devices (human interface).

  - Events are notifications that something has changed, eg. button pressed

 */

#ifndef INPUT_GP_EVENT_H
#define INPUT_GP_EVENT_H

#include <stdint.h>
#include <sys/time.h>

#include <input/gp_types.h>
#include <input/gp_event_keys.h>

enum gp_event_type {
	GP_EV_KEY = 1, /* key/button press event */
	GP_EV_REL = 2, /* relative event */
	GP_EV_ABS = 3, /* absolute event */
	GP_EV_SYS = 4, /* system events window close, resize... */
	GP_EV_TMR = 5, /* timer expired event */
	GP_EV_MAX = 5, /* maximum, greater values are free */
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
	char ascii;
};

struct gp_ev_sys {
	uint32_t w, h;
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
		/* timer event */
		gp_timer *tmr;
	};

	/* event timestamp */
	struct timeval time;

	/*
	 * Cursor position, position on screen accumulated
	 * from all pointer devices
	 */
	uint32_t cursor_x;
	uint32_t cursor_y;

	void *priv;
};

/*
 * Dump event into stdout.
 */
void gp_event_dump(gp_event *ev);

/*
 * Returns human-readable key name.
 */
const char *gp_event_key_name(enum gp_event_key_value key);

/*
 * Retruns difference between two timeval structures in miliseconds
 */
static inline int gp_timeval_diff_ms(struct timeval a, struct timeval b)
{
	return (a.tv_sec - b.tv_sec) * 1000 +
	       (a.tv_usec - b.tv_usec + 500) / 1000;
}

#endif /* INPUT_GP_EVENT_H */
