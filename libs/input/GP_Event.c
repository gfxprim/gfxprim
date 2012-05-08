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
 * Copyright (C) 2009-2011 Cyril Hrubis <metan@ucw.cz>                       *
 *                                                                           *
 *****************************************************************************/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "GP_Common.h"
#include "GP_Event.h"

/* Screen size for clipping the cursor possition */
static uint32_t screen_w = 0, screen_h = 0;

/* Event queue */
static uint32_t queue_first = 0, queue_last = 0; 
static struct GP_Event event_queue[GP_EVENT_QUEUE_SIZE];

/* Global input state */
static struct GP_Event cur_state = {.cursor_x = 0, .cursor_y = 0};

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
 "Insert",      "Delete",    "?",          "?",            "?",
 "?",           "?",         "KP Equal",   "KP PlusMinus", "Pause",
 "?",           "KP Comma",  "?",          "?",            "?",
 "LeftMeta",    "RightMeta", "Compose",
};

static uint16_t key_names_size = sizeof(key_names)/sizeof(void*);

void GP_EventSetScreenSize(uint32_t w, uint32_t h)
{
	screen_w = w;
	screen_h = h;

	/* clip cursor */
	if (cur_state.cursor_x >= w)
		cur_state.cursor_x = w - 1;
	
	if (cur_state.cursor_y >= h)
		cur_state.cursor_y = h - 1;
}

void GP_EventSetScreenCursor(uint32_t x, uint32_t y)
{
	cur_state.cursor_x = x;
	cur_state.cursor_y = y;
}

uint32_t GP_EventQueued(void)
{
	if (queue_first <= queue_last)
		return queue_last - queue_first;

	return GP_EVENT_QUEUE_SIZE - (queue_last - queue_first);
}

int GP_EventGet(struct GP_Event *ev)
{
	if (queue_first == queue_last)
		return 0;

	*ev = event_queue[queue_first];

	queue_first = (queue_first + 1) % GP_EVENT_QUEUE_SIZE;

	return 1;
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
	default:
		return "Unknown";
	};

}

static void dump_rel(struct GP_Event *ev)
{
	printf("Rel ");

	switch (ev->code) {
	case GP_EV_REL_POS:
		printf("Position %u %u\n", ev->cursor_x, ev->cursor_y);
	break;
	case GP_EV_REL_WHEEL:
		printf("Wheel %i\n", ev->val.val);
	break;
	}
}

static void dump_key(struct GP_Event *ev)
{
	const char *name = GP_EventKeyName(ev->val.key.key);

	if (ev->val.key.key < key_names_size)
		name = key_names[ev->val.key.key];

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
	default:
		printf("Unknown %u\n", ev->type);
	}
}

static void event_put(struct GP_Event *ev)
{
	uint32_t next = (queue_last + 1) % GP_EVENT_QUEUE_SIZE;

	if (next == queue_first) {
		fprintf(stderr, "Event queue full, dropping event\n");
		return;
	}
	
	event_queue[queue_last] = *ev;
	queue_last = next;
}

static void set_time(struct timeval *time)
{
	if (time == NULL)
		gettimeofday(&cur_state.time, NULL);
	else
		cur_state.time = *time;
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
	int32_t rx = x - cur_state.cursor_x;
	int32_t ry = y - cur_state.cursor_y;

	GP_EventPushRel(rx, ry, time);
}

void GP_EventPushRel(int32_t rx, int32_t ry, struct timeval *time)
{
	/* event header */
	cur_state.type  = GP_EV_REL;
	cur_state.code  = GP_EV_REL_POS;

	cur_state.val.rel.rx = rx;
	cur_state.val.rel.ry = ry;

 	set_time(time);

	/* move the global cursor */
	cur_state.cursor_x = clip_rel(cur_state.cursor_x, screen_w, rx); 
	cur_state.cursor_y = clip_rel(cur_state.cursor_y, screen_h, ry); 

	/* put it into queue */
	event_put(&cur_state);
}

void GP_EventPushAbs(uint32_t x, uint32_t y, uint32_t pressure,
                     uint32_t x_max, uint32_t y_max, uint32_t pressure_max,
                     struct timeval *time)
{
	/* event header */
	cur_state.type  = GP_EV_ABS;
	cur_state.code  = GP_EV_ABS_POS;
	cur_state.val.abs.x = x; 
	cur_state.val.abs.y = y;
	cur_state.val.abs.pressure = pressure;
	cur_state.val.abs.x_max = x_max; 
	cur_state.val.abs.y_max = y_max;
	cur_state.val.abs.pressure_max = pressure_max;
	
	set_time(time);

	/* 
	 * Set global cursor, the packet could be partial, eg. update only x or
	 * only y. In such case x_max or y_max is zero.
	 */

	if (x_max != 0)	
		cur_state.cursor_x = x * (screen_w - 1) / x_max;
	
	if (y_max != 0)
		cur_state.cursor_y = y * (screen_h - 1) / y_max;

	/* put it into queue */
	event_put(&cur_state);
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
		GP_EventResetKey(&cur_state, key);	
	break;
	case GP_EV_KEY_DOWN:
		GP_EventSetKey(&cur_state, key);	
	break;
	case GP_EV_KEY_REPEAT:
	break;
	default:
		fprintf(stderr, "Invalid key event code %u\n", code);
		return;
	}

	/* event header */
	cur_state.type  = GP_EV_KEY;
	cur_state.code  = code;
	cur_state.val.key.key = key;
	key_to_ascii(&cur_state);
	
	set_time(time);

	/* put it into queue  */
	event_put(&cur_state);
}

void GP_EventPush(uint16_t type, uint32_t code, int32_t value,
                  struct timeval *time)
{
	switch (type) {
	case GP_EV_KEY:
		GP_EventPushKey(code, value, time);
	break;
	default:
		cur_state.type  = type;
		cur_state.code  = code;
		cur_state.val.val = value;

		set_time(time);

		event_put(&cur_state);
	}
}
