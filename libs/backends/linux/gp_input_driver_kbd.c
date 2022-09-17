// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2009-2013 Cyril Hrubis <metan@ucw.cz>
 */

#include "core/gp_common.h"
#include <core/gp_debug.h>

#include <input/gp_event_queue.h>
#include <input/gp_input_driver_kbd.h>

/* KBD raw mode keycodes */
static uint16_t keycode_table[] = {
	GP_KEY_ESC,        GP_KEY_1,           GP_KEY_2,           GP_KEY_3,
	GP_KEY_4,          GP_KEY_5,           GP_KEY_6,           GP_KEY_7,
	GP_KEY_8,          GP_KEY_9,           GP_KEY_0,           GP_KEY_MINUS,
	GP_KEY_EQUAL,      GP_KEY_BACKSPACE,   GP_KEY_TAB,         GP_KEY_Q,
	GP_KEY_W,          GP_KEY_E,           GP_KEY_R,           GP_KEY_T,
	GP_KEY_Y,          GP_KEY_U,           GP_KEY_I,           GP_KEY_O,
	GP_KEY_P,          GP_KEY_LEFT_BRACE,  GP_KEY_RIGHT_BRACE, GP_KEY_ENTER,
	GP_KEY_LEFT_CTRL,  GP_KEY_A,           GP_KEY_S,           GP_KEY_D,
	GP_KEY_F,          GP_KEY_G,           GP_KEY_H,           GP_KEY_J,
	GP_KEY_K,          GP_KEY_L,           GP_KEY_SEMICOLON,   GP_KEY_APOSTROPHE,
	GP_KEY_GRAVE,      GP_KEY_LEFT_SHIFT,  GP_KEY_BACKSLASH,   GP_KEY_Z,
	GP_KEY_X,          GP_KEY_C,           GP_KEY_V,           GP_KEY_B,
	GP_KEY_N,          GP_KEY_M,           GP_KEY_COMMA,       GP_KEY_DOT,
	GP_KEY_SLASH,      GP_KEY_RIGHT_SHIFT, GP_KEY_KP_ASTERISK, GP_KEY_LEFT_ALT,
	GP_KEY_SPACE,      GP_KEY_CAPS_LOCK,   GP_KEY_F1,          GP_KEY_F2,
	GP_KEY_F3,         GP_KEY_F4,          GP_KEY_F5,          GP_KEY_F6,
	GP_KEY_F7,         GP_KEY_F8,          GP_KEY_F9,          GP_KEY_F10,
	GP_KEY_NUM_LOCK,   GP_KEY_SCROLL_LOCK, GP_KEY_KP_7,        GP_KEY_KP_8,
	GP_KEY_KP_MINUS,   GP_KEY_KP_9,        GP_KEY_KP_4,        GP_KEY_KP_5,
	GP_KEY_KP_6,       GP_KEY_KP_PLUS,     GP_KEY_KP_1,        GP_KEY_KP_2,
	GP_KEY_KP_3,       GP_KEY_KP_0,        GP_KEY_KP_COMMA,    0,
	0,                 0,                  GP_KEY_F11,         GP_KEY_F12,
	0,                 0,                  0,                  0,
	0,                 0,                  0,                  GP_KEY_KP_ENTER,
	GP_KEY_RIGHT_CTRL, GP_KEY_KP_ASTERISK, GP_KEY_SYSRQ,       GP_KEY_RIGHT_ALT,
	0,                 GP_KEY_HOME,        GP_KEY_UP,          GP_KEY_PAGE_UP,
	GP_KEY_LEFT,       GP_KEY_RIGHT,       GP_KEY_END,         GP_KEY_DOWN,
	GP_KEY_PAGE_DOWN,  GP_KEY_INSERT,      GP_KEY_DELETE,      0,
	0,                 0,                  0,                  0,
	0,                 0,                  0,                  0,
	0,                 0,                  0,                  0,
	GP_KEY_LEFT_META,
};

void gp_input_driver_kbd_event_put(gp_event_queue *event_queue,
                                   unsigned char ev)
{
	unsigned int keycode = ev & 0x7f;
	int press = !(ev & 0x80);
	int key;

	GP_DEBUG(2, "Press %i keycode %i", press, keycode);

	if (keycode > 0 && keycode <= GP_ARRAY_SIZE(keycode_table)) {
		key = keycode_table[keycode - 1];

		if (key != 0) {
			gp_event_queue_push_key(event_queue, key, press, 0);
			return;
		}
	}

	GP_WARN("Unmapped key %i", keycode);
}
