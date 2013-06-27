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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <core/GP_Debug.h>
#include <core/GP_Common.h>

#include <input/GP_EventQueue.h>

#include <input/GP_Event.h>

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

const char *GP_EventKeyName(enum GP_EventKeyValue key)
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

static void dump_rel(struct GP_Event *ev)
{
	printf("Rel ");

	switch (ev->code) {
	case GP_EV_REL_POS:
		printf("Position %u %u dx=%i dy=%i\n",
		       ev->cursor_x, ev->cursor_y,
		       ev->val.rel.rx, ev->val.rel.ry);
	break;
	case GP_EV_REL_WHEEL:
		printf("Wheel %i\n", ev->val.val);
	break;
	}
}

static void dump_key(struct GP_Event *ev)
{
	const char *name = GP_EventKeyName(ev->val.key.key);

	printf("Key %i (Key%s) %s\n",
	       ev->val.key.key, name, ev->code ? "down" : "up");
}

static void dump_abs(struct GP_Event *ev)
{
	switch (ev->code) {
	case GP_EV_ABS_POS:
		printf("Position %u %u %u\n",
		       ev->cursor_x, ev->cursor_y, ev->val.abs.pressure);
	break;
	}
}

static void dump_sys(struct GP_Event *ev)
{
	switch (ev->code) {
	case GP_EV_SYS_QUIT:
		printf("Sys Quit\n");
	break;
	case GP_EV_SYS_RESIZE:
		printf("Sys Resize %ux%u\n", ev->val.sys.w, ev->val.sys.h);
	break;
	}
}

void GP_EventDump(struct GP_Event *ev)
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
		printf("Timer %s expired\n", ev->val.tmr->id);
	break;
	default:
		printf("Unknown %u\n", ev->type);
	}
}
