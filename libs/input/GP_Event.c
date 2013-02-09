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

#include "GP_Event.h"

static GP_EVENT_QUEUE_DECLARE(event_queue, 0, 0);

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

static uint16_t key_names_size = sizeof(key_names)/sizeof(void*);

void GP_EventSetScreenSize(uint32_t w, uint32_t h)
{
	GP_EventQueueSetScreenSize(&event_queue, w, h);
}

void GP_EventSetScreenCursor(uint32_t x, uint32_t y)
{
	GP_EventQueueSetCursorPosition(&event_queue, x, y);
}

unsigned int GP_EventsQueued(void)
{
	return GP_EventQueueEventsQueued(&event_queue);
}

int GP_EventGet(struct GP_Event *ev)
{
	return GP_EventQueueGetEvent(&event_queue, ev);
}

const char *GP_EventKeyName(enum GP_EventKeyValue key)
{
	if (key < key_names_size)
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
	default:
		printf("Unknown %u\n", ev->type);
	}
}

static void event_put(struct GP_Event *ev)
{
	GP_EventQueuePutEvent(&event_queue, ev);
}

static void set_time(struct timeval *time)
{
	if (time == NULL)
		gettimeofday(&event_queue.cur_state.time, NULL);
	else
		event_queue.cur_state.time = *time;
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

void GP_EventPushRelTo(uint32_t x, uint32_t y, struct timeval *time)
{
	if (x > event_queue.screen_w || y > event_queue.screen_h) {
		GP_WARN("x > screen_w or y > screen_h, forgot to set screen size?");
		return;
	}
	
	int32_t rx = x - event_queue.cur_state.cursor_x;
	int32_t ry = y - event_queue.cur_state.cursor_y;

	GP_EventPushRel(rx, ry, time);
}

void GP_EventPushRel(int32_t rx, int32_t ry, struct timeval *time)
{
	/* event header */
	event_queue.cur_state.type  = GP_EV_REL;
	event_queue.cur_state.code  = GP_EV_REL_POS;

	event_queue.cur_state.val.rel.rx = rx;
	event_queue.cur_state.val.rel.ry = ry;

 	set_time(time);

	/* move the global cursor */
	event_queue.cur_state.cursor_x = clip_rel(event_queue.cur_state.cursor_x, event_queue.screen_w, rx); 
	event_queue.cur_state.cursor_y = clip_rel(event_queue.cur_state.cursor_y, event_queue.screen_h, ry); 

	/* put it into queue */
	event_put(&event_queue.cur_state);
}

void GP_EventPushAbs(uint32_t x, uint32_t y, uint32_t pressure,
                     uint32_t x_max, uint32_t y_max, uint32_t pressure_max,
                     struct timeval *time)
{
	/* event header */
	event_queue.cur_state.type  = GP_EV_ABS;
	event_queue.cur_state.code  = GP_EV_ABS_POS;
	event_queue.cur_state.val.abs.x = x; 
	event_queue.cur_state.val.abs.y = y;
	event_queue.cur_state.val.abs.pressure = pressure;
	event_queue.cur_state.val.abs.x_max = x_max; 
	event_queue.cur_state.val.abs.y_max = y_max;
	event_queue.cur_state.val.abs.pressure_max = pressure_max;
	
	set_time(time);

	/* 
	 * Set global cursor, the packet could be partial, eg. update only x or
	 * only y. In such case x_max or y_max is zero.
	 */
	if (x_max != 0)	
		event_queue.cur_state.cursor_x = x * (event_queue.screen_w - 1) / x_max;
	
	if (y_max != 0)
		event_queue.cur_state.cursor_y = y * (event_queue.screen_h - 1) / y_max;

	/* put it into queue */
	event_put(&event_queue.cur_state);
}

void GP_EventPushResize(uint32_t w, uint32_t h, struct timeval *time)
{
	/* event header */
	event_queue.cur_state.type  = GP_EV_SYS;
	event_queue.cur_state.code  = GP_EV_SYS_RESIZE;

	event_queue.cur_state.val.sys.w = w;
	event_queue.cur_state.val.sys.h = h;

 	set_time(time);

	/* put it into queue */
	event_put(&event_queue.cur_state);
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

static char keys_to_ascii_s[] = {
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
static void key_to_ascii(GP_Event *ev)
{
	ev->val.key.ascii = 0;
	
	if (GP_EventGetKey(ev, GP_KEY_LEFT_SHIFT) ||
	    GP_EventGetKey(ev, GP_KEY_RIGHT_SHIFT)) {
		if (ev->val.key.key < sizeof(keys_to_ascii_s))
			ev->val.key.ascii = keys_to_ascii_s[ev->val.key.key];
	} else {
		if (ev->val.key.key < sizeof(keys_to_ascii))
			ev->val.key.ascii = keys_to_ascii[ev->val.key.key];
	}
}

void GP_EventPushKey(uint32_t key, uint8_t code, struct timeval *time)
{
	switch (code) {
	case GP_EV_KEY_UP:
		GP_EventResetKey(&event_queue.cur_state, key);	
	break;
	case GP_EV_KEY_DOWN:
		GP_EventSetKey(&event_queue.cur_state, key);	
	break;
	case GP_EV_KEY_REPEAT:
	break;
	default:
		GP_WARN("Invalid key event code %u", code);
		return;
	}

	/* event header */
	event_queue.cur_state.type  = GP_EV_KEY;
	event_queue.cur_state.code  = code;
	event_queue.cur_state.val.key.key = key;
	key_to_ascii(&event_queue.cur_state);
	
	set_time(time);

	/* put it into queue  */
	event_put(&event_queue.cur_state);
}

void GP_EventPush(uint16_t type, uint32_t code, int32_t value,
                  struct timeval *time)
{
	switch (type) {
	case GP_EV_KEY:
		GP_EventPushKey(code, value, time);
	break;
	default:
		event_queue.cur_state.type  = type;
		event_queue.cur_state.code  = code;
		event_queue.cur_state.val.val = value;

		set_time(time);

		event_put(&event_queue.cur_state);
	}
}
