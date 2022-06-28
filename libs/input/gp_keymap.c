// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2009-2022 Cyril Hrubis <metan@ucw.cz>
 */

#include <string.h>
#include <core/gp_debug.h>
#include <input/gp_event.h>
#include <input/gp_event_queue.h>
#include <input/gp_keymap.h>

struct gp_keymap_us {
	int (*event_key)(gp_keymap *self, gp_event_queue *queue, gp_event *ev);
	uint8_t lshift_state;
	uint8_t rshift_state;
	uint8_t caps_state;
};

static char keys_to_ascii[] = {
	   0x00, 0x00,  '1',  '2',  '3',  '4',  '5',  '6',  '7',  '8',
	    '9',  '0',  '-',  '=', 0x08, '\t',  'q',  'w',  'e',  'r',
	    't',  'y',  'u',  'i',  'o',  'p',  '[',  ']', '\n', 0x00,
	    'a',  's',  'd',  'f',  'g',  'h',  'j',  'k',  'l',  ';',
	   '\'',  '`', 0x00, '\\',  'z',  'x',  'c',  'v',  'b',  'n',
	    'm',  ',',  '.',  '/', 0x00,  '*', 0x00,  ' ', 0x00, 0x00,
	   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	   0x00,  '7',  '8',  '9',  '-',  '4',  '5',  '6',  '+',  '1',
	    '2',  '3',  '0',  '.'
};

static char keys_to_ascii_shift[] = {
	   0x00, 0x00,  '!',  '@',  '#',  '$',  '%',  '^',  '&',  '*',
	    '(',  ')',  '_',  '+', 0x08, '\t',  'Q',  'W',  'E',  'R',
	    'T',  'Y',  'U',  'I',  'O',  'P',  '{',  '}', '\n', 0x00,
	    'A',  'S',  'D',  'F',  'G',  'H',  'J',  'K',  'L',  ':',
	    '"',  '~', 0x00,  '|',  'Z',  'X',  'C',  'V',  'B',  'N',
	    'M',  '<',  '>',  '?', 0x00,  '*', 0x00,  ' ', 0x00, 0x00,
	   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	   0x00,  '7',  '8',  '9',  '-',  '4',  '5',  '6',  '+',  '1',
	    '2',  '3',  '0',  '.'
};

static int event_key_us(gp_keymap *self, gp_event_queue *queue, gp_event *ev)
{
	unsigned int key = ev->key.key;
	struct gp_keymap_us *keymap_us = (void*)self;
	uint32_t utf;

	switch (key) {
	case GP_KEY_LEFT_SHIFT:
		keymap_us->lshift_state = ev->code;
	break;
	case GP_KEY_RIGHT_SHIFT:
		keymap_us->rshift_state = ev->code;
	break;
	case GP_KEY_CAPS_LOCK:
		if (ev->code == GP_EV_KEY_DOWN)
			keymap_us->caps_state = !keymap_us->caps_state;
	break;
	}

	if (ev->code != GP_EV_KEY_DOWN)
		return 0;

	if (key >= sizeof(keys_to_ascii))
		return 0;

	uint8_t uppecase = keymap_us->lshift_state ||
	                   keymap_us->rshift_state ||
			   keymap_us->caps_state;

	if (uppecase)
		utf = keys_to_ascii_shift[key];
	else
		utf = keys_to_ascii[key];

	if (utf >= 0x20)
		gp_event_queue_push_utf(queue, utf, &ev->time);

	return 0;
}

static struct gp_keymap_us keymap_us = {
	.event_key = event_key_us,
};

gp_keymap *gp_keymap_load(const char *name)
{
	GP_DEBUG(1, "Loading '%s' keymap", name ? name : "default");

	if (!name || !strcmp(name, "us"))
		return (gp_keymap*)&keymap_us;

	GP_WARN("Keymap '%s' not found", name);

	return (gp_keymap*)&keymap_us;
}
