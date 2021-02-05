// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2009-2013 Cyril Hrubis <metan@ucw.cz>
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <core/gp_debug.h>
#include "core/gp_common.h"

#include <input/gp_event_queue.h>
#include <input/gp_timer.h>
#include <input/gp_event.h>

static char *key_names[] = {
 "Reserved",    "Escape",    "1",          "2",            "3",
 "4",           "5",         "6",          "7",            "8",
 "9",           "0",         "Minus",      "Equal",        "BackSpace",
 "Tab",         "Q",         "W",          "E",            "R",
 "T",           "Y",         "U",          "I",            "O",
 "P",           "LeftBrace", "RightBrace", "Enter",        "LeftCtrl",
 "A",           "S",         "D",          "F",            "G",
 "H",           "J",         "K",          "L",            "Semicolon",
 "Apostrophe",  "Grave",     "LeftShift",  "BackSlash",    "Z",
 "X",           "C",         "V",          "B",            "N",
 "M",           "Comma",     "Dot",        "Slash",        "RightShift",
 "KP Asterisk", "LeftAlt",   "Space",      "CapsLock",     "F1",
 "F2",          "F3",        "F4",         "F5",           "F6",
 "F7",          "F8",        "F9",         "F10",          "NumLock",
 "ScrollLock",  "KP 7",      "KP 8",       "KP 9",         "KP Minus",
 "KP 4",        "KP 5",      "KP 6",       "KP Plus",      "KP 1",
 "KP 2",        "KP 3",      "KP 0",       "KP Dot",       "?",
 "?",           "?",         "F11",        "F12",          "?",
 "?",           "?",         "?",          "?",            "?",
 "?",           "KP Enter",  "RightCtrl",  "KP Slash",     "SysRq",
 "RightAlt",    "?",         "Home",       "Up",           "PageUp",
 "Left",        "Right",     "End",        "Down",         "PageDown",
 "Insert",      "Delete",    "?",          "Mute",         "VolumeDown",
 "VolumeUp",    "?",         "KP Equal",   "KP PlusMinus", "Pause",
 "?",           "KP Comma",  "?",          "?",            "?",
 "LeftMeta",    "RightMeta", "Compose",
};

const char *gp_event_key_name(enum gp_event_key_value key)
{
	if (key < GP_ARRAY_SIZE(key_names))
		return key_names[key];

	switch (key) {
	case GP_BTN_LEFT:
		return "LeftButton";
	case GP_BTN_RIGHT:
		return "RightButton";
	case GP_BTN_MIDDLE:
		return "MiddleButton";
	case GP_BTN_PEN:
		return "Pen";
	case GP_KEY_NEXTSONG:
		return "NextSong";
	case GP_KEY_PREVIOUSSONG:
		return "PreviousSong";
	case GP_KEY_PLAYPAUSE:
		return "PlayPause";
	default:
		return "Unknown";
	};
}

static void dump_rel(gp_event *ev)
{
	printf("Rel ");

	switch (ev->code) {
	case GP_EV_REL_POS:
		printf("Position %u %u dx=%i dy=%i\n",
		       ev->cursor_x, ev->cursor_y,
		       ev->rel.rx, ev->rel.ry);
	break;
	case GP_EV_REL_WHEEL:
		printf("Wheel %i\n", ev->val);
	break;
	}
}

static void dump_key(gp_event *ev)
{
	const char *name = gp_event_key_name(ev->key.key);

	printf("Key %i (Key%s) %s\n",
	       ev->key.key, name, ev->code ? "down" : "up");
}

static void dump_abs(gp_event *ev)
{
	switch (ev->code) {
	case GP_EV_ABS_POS:
		printf("Position %u %u %u\n",
		       ev->cursor_x, ev->cursor_y, ev->abs.pressure);
	break;
	}
}

static void dump_sys(gp_event *ev)
{
	switch (ev->code) {
	case GP_EV_SYS_QUIT:
		printf("Sys Quit\n");
	break;
	case GP_EV_SYS_RESIZE:
		printf("Sys Resize %ux%u\n", ev->sys.w, ev->sys.h);
	break;
	}
}

void gp_event_dump(gp_event *ev)
{
	printf("Event (%u) ", (unsigned int)ev->time.tv_sec % 10000);

	switch (ev->type) {
	case GP_EV_KEY:
		dump_key(ev);
	break;
	case GP_EV_REL:
		dump_rel(ev);
	break;
	case GP_EV_ABS:
		dump_abs(ev);
	break;
	case GP_EV_SYS:
		dump_sys(ev);
	break;
	case GP_EV_TMR:
		printf("Timer %s expired\n", ev->tmr->id);
	break;
	default:
		printf("Unknown %u\n", ev->type);
	}
}
