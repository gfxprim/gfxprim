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

  Gfxprim event layer.

  Events are lowlevel interface to input devices (human interface).

  - Events are notifications that something has changed, eg. button pressed
  - Each event carries some information about global state

 */

#ifndef INPUT_GP_EVENT_H
#define INPUT_GP_EVENT_H

#include <stdint.h>
#include <sys/time.h>

#include <input/GP_Types.h>

#define GP_EVENT_QUEUE_SIZE 32

#define GP_EVENT_KEYMAP_BYTES 36

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

/*
 * This is 1:1 to linux kernel input subsystem.
 */
enum gp_event_key_value {
	GP_KEY_ESC            =  1,
	GP_KEY_1              =  2,
	GP_KEY_2              =  3,
	GP_KEY_3              =  4,
	GP_KEY_4              =  5,
	GP_KEY_5              =  6,
	GP_KEY_6              =  7,
	GP_KEY_7              =  8,
	GP_KEY_8              =  9,
	GP_KEY_9              = 10,
	GP_KEY_0              = 11,
	GP_KEY_MINUS          = 12,
	GP_KEY_EQUAL          = 13,
	GP_KEY_BACKSPACE      = 14,
	GP_KEY_TAB            = 15,
	GP_KEY_Q              = 16,
	GP_KEY_W              = 17,
	GP_KEY_E              = 18,
	GP_KEY_R              = 19,
	GP_KEY_T              = 20,
	GP_KEY_Y              = 21,
	GP_KEY_U              = 22,
	GP_KEY_I              = 23,
	GP_KEY_O              = 24,
	GP_KEY_P              = 25,
	GP_KEY_LEFT_BRACE     = 26,
	GP_KEY_RIGHT_BRACE    = 27,
	GP_KEY_ENTER          = 28,
	GP_KEY_LEFT_CTRL      = 29,
	GP_KEY_A              = 30,
	GP_KEY_S              = 31,
	GP_KEY_D              = 32,
	GP_KEY_F              = 33,
	GP_KEY_G              = 34,
	GP_KEY_H              = 35,
	GP_KEY_J              = 36,
	GP_KEY_K              = 37,
	GP_KEY_L              = 38,
	GP_KEY_SEMICOLON      = 39,
	GP_KEY_APOSTROPHE     = 40,
	GP_KEY_GRAVE          = 41,
	GP_KEY_LEFT_SHIFT     = 42,
	GP_KEY_BACKSLASH      = 43,
	GP_KEY_Z              = 44,
	GP_KEY_X              = 45,
	GP_KEY_C              = 46,
	GP_KEY_V              = 47,
	GP_KEY_B              = 48,
	GP_KEY_N              = 49,
	GP_KEY_M              = 50,
	GP_KEY_COMMA          = 51,
	GP_KEY_DOT            = 52,
	GP_KEY_SLASH          = 53,
	GP_KEY_RIGHT_SHIFT    = 54,
	GP_KEY_KP_ASTERISK    = 55,
	GP_KEY_LEFT_ALT       = 56,
	GP_KEY_SPACE          = 57,
	GP_KEY_CAPS_LOCK      = 58,
	GP_KEY_F1             = 59,
	GP_KEY_F2             = 60,
	GP_KEY_F3             = 61,
	GP_KEY_F4             = 62,
	GP_KEY_F5             = 63,
	GP_KEY_F6             = 64,
	GP_KEY_F7             = 65,
	GP_KEY_F8             = 66,
	GP_KEY_F9             = 67,
	GP_KEY_F10            = 68,
	GP_KEY_NUM_LOCK       = 69,
	GP_KEY_SCROLL_LOCK    = 70,
	GP_KEY_KP_7           = 71,
	GP_KEY_KP_8           = 72,
	GP_KEY_KP_9           = 73,
	GP_KEY_KP_MINUS       = 74,
	GP_KEY_KP_4           = 75,
	GP_KEY_KP_5           = 76,
	GP_KEY_KP_6           = 77,
	GP_KEY_KP_PLUS        = 78,
	GP_KEY_KP_1           = 79,
	GP_KEY_KP_2           = 80,
	GP_KEY_KP_3           = 81,
	GP_KEY_KP_0           = 82,
	GP_KEY_KP_DOT         = 83,

	GP_KEY_F11            = 87,
	GP_KEY_F12            = 88,

	GP_KEY_KP_ENTER       = 96,
	GP_KEY_RIGHT_CTRL     = 97,
	GP_KEY_KP_SLASH       = 98,
	GP_KEY_SYSRQ          = 99,
	GP_KEY_RIGHT_ALT      = 100,

	GP_KEY_HOME           = 102,
	GP_KEY_UP             = 103,
	GP_KEY_PAGE_UP        = 104,
	GP_KEY_LEFT           = 105,
	GP_KEY_RIGHT          = 106,
	GP_KEY_END            = 107,
	GP_KEY_DOWN           = 108,
	GP_KEY_PAGE_DOWN      = 109,
	GP_KEY_INSERT         = 110,
	GP_KEY_DELETE         = 111,

	GP_KEY_MUTE           = 113,
	GP_KEY_VOLUMEDOWN     = 114,
	GP_KEY_VOLUMEUP       = 115,

	GP_KEY_KP_EQUAL       = 117,
	GP_KEY_KP_PLUS_MINUS  = 118,
	GP_KEY_PAUSE          = 119,

	GP_KEY_KP_COMMA       = 121,

	GP_KEY_LEFT_META      = 125,
	GP_KEY_RIGHT_META     = 126,
	GP_KEY_COMPOSE        = 127,

	GP_KEY_NEXTSONG       = 163,
	GP_KEY_PLAYPAUSE      = 164,
	GP_KEY_PREVIOUSSONG   = 165,

	GP_KEY_F13            = 183,
	GP_KEY_F14            = 184,
	GP_KEY_F15            = 185,
	GP_KEY_F16            = 186,
	GP_KEY_F17            = 187,
	GP_KEY_F18            = 188,
	GP_KEY_F19            = 189,
	GP_KEY_F20            = 190,
	GP_KEY_F21            = 191,
	GP_KEY_F22            = 192,
	GP_KEY_F23            = 193,
	GP_KEY_F24            = 194,

	/* Common Buttons */
	GP_BTN_0              = 0x100,
	GP_BTN_1              = 0x101,
	GP_BTN_2              = 0x102,
	GP_BTN_3              = 0x103,
	GP_BTN_4              = 0x104,
	GP_BTN_5              = 0x105,
	GP_BTN_6              = 0x106,
	GP_BTN_7              = 0x107,
	GP_BTN_8              = 0x108,
	GP_BTN_9              = 0x109,

	/* Mouse Buttons */
	GP_BTN_LEFT           = 0x110,
	GP_BTN_RIGHT          = 0x111,
	GP_BTN_MIDDLE         = 0x112,
	GP_BTN_SIDE           = 0x113,
	GP_BTN_EXTRA          = 0x114,
	GP_BTN_FORWARD        = 0x115,
	GP_BTN_BACK           = 0x116,
	GP_BTN_TASK           = 0x117,

	/* Touchscreen pen */
	GP_BTN_PEN            = 0x14a,
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

union gp_ev_val {
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

struct gp_event {
	/* event */
	uint16_t type;
	uint32_t code;
	union gp_ev_val val;

	/* input device id */
	uint32_t dev_id;

	/* event timestamp */
	struct timeval time;

	/*
	 * Cursor position, position on screen accumulated
	 * from all pointer devices
	 */
	uint32_t cursor_x;
	uint32_t cursor_y;

	/*
	 * Bitmap of pressed keys including mouse buttons
	 * accumulated for all input devices.
	 */
	uint8_t keys_pressed[GP_EVENT_KEYMAP_BYTES];
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
 * Helpers for setting/getting key bits.
 */
static inline void gp_event_set_key(gp_event *ev, uint32_t key)
{
	if (key >= GP_EVENT_KEYMAP_BYTES * 8)
		return;

	ev->keys_pressed[(key)/8] |= 1<<((key)%8);
}

static inline int gp_event_get_key(gp_event *ev, uint32_t key)
{
	if (key >= GP_EVENT_KEYMAP_BYTES * 8)
		return 0;

	return !!(ev->keys_pressed[(key)/8] & (1<<((key)%8)));
}

static inline void gp_event_reset_key(gp_event *ev, uint32_t key)
{
	if (key >= GP_EVENT_KEYMAP_BYTES * 8)
		return;

	ev->keys_pressed[(key)/8] &= ~(1<<((key)%8));
}

#endif /* INPUT_GP_EVENT_H */
