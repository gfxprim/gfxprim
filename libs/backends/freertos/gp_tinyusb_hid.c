// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2022 Cyril Hrubis <metan@ucw.cz>
 */

#include <input/gp_ev_queue.h>
#include <core/gp_common.h>
#include <core/gp_debug.h>
#include <inttypes.h>
#include <tusb.h>

extern gp_ev_queue *gp_rtos_ev_queue;

struct kbd_feedback {
	uint8_t dev_addr;
	uint8_t instance;
	uint8_t leds;
	gp_ev_feedback feedback;
};

static int kbd_feedback_set(gp_ev_feedback *self, gp_ev_feedback_op *op)
{
	struct kbd_feedback *kbd_feedback = GP_CONTAINER_OF(self, struct kbd_feedback, feedback);

	switch (op->op) {
	case GP_EV_LEDS_ON:
		kbd_feedback->leds |= op->val;
	break;
	case GP_EV_LEDS_OFF:
		kbd_feedback->leds &= ~(op->val);
	break;
	case GP_EV_LEDS_GET:
		op->val = kbd_feedback->leds;
		return 0;
	default:
		return -1;
	}

	tuh_hid_set_report(kbd_feedback->dev_addr, kbd_feedback->instance,
                           0, HID_REPORT_TYPE_OUTPUT, &kbd_feedback->leds, 1);

	return 0;
}

static struct kbd_feedback kbd_feedback = {
	.feedback = {.set_get = kbd_feedback_set},
};

static void kbd_feedback_register(uint8_t dev_addr, uint8_t instance)
{
	kbd_feedback.dev_addr = dev_addr;
	kbd_feedback.instance = instance;

	gp_ev_queue_feedback_register(gp_rtos_ev_queue, &kbd_feedback.feedback);
}

static void kbd_feedback_unregister(uint8_t dev_addr, uint8_t instance)
{
	if (kbd_feedback.dev_addr != dev_addr || kbd_feedback.instance != instance)
		return;

	gp_ev_queue_feedback_unregister(gp_rtos_ev_queue, &kbd_feedback.feedback);
}

void tuh_hid_mount_cb(uint8_t dev_addr, uint8_t instance,
                      uint8_t const* desc_report, uint16_t desc_len)
{
	uint8_t protocol = tuh_hid_interface_protocol(dev_addr, instance);
	const char *str_prot = "Invalid";

	switch (protocol) {
	case HID_ITF_PROTOCOL_NONE:
		str_prot = "None";
	break;
	case HID_ITF_PROTOCOL_KEYBOARD:
		str_prot = "Keyboard";
		kbd_feedback_register(dev_addr, instance);
	break;
	case HID_ITF_PROTOCOL_MOUSE:
		str_prot = "Mouse";
	break;
	}

	GP_DEBUG(1, "HID device plugged (%s) addr=%" PRIu8 "inst=%" PRIu8,
	         str_prot, dev_addr, instance);

	if (protocol == HID_ITF_PROTOCOL_NONE)
		return;

	if (!tuh_hid_receive_report(dev_addr, instance))
		GP_WARN("Cannot enable reports!");
}

static struct mod_key {
	uint8_t mask;
	uint8_t key;
} mod_keys[] = {
	{0x01, GP_KEY_LEFT_CTRL},
	{0x02, GP_KEY_LEFT_SHIFT},
	{0x04, GP_KEY_LEFT_ALT},
	{0x08, GP_KEY_LEFT_META},
	{0x10, GP_KEY_RIGHT_CTRL},
	{0x20, GP_KEY_RIGHT_SHIFT},
	{0x40, GP_KEY_RIGHT_ALT},
	{0x80, GP_KEY_RIGHT_META},
};

static uint16_t keymap[0xff] = {
	0, 0, 0, 0,
	GP_KEY_A, GP_KEY_B, GP_KEY_C, GP_KEY_D, GP_KEY_E,
	GP_KEY_F, GP_KEY_G, GP_KEY_H, GP_KEY_I, GP_KEY_J,
	GP_KEY_K, GP_KEY_L, GP_KEY_M, GP_KEY_N, GP_KEY_O,
	GP_KEY_P, GP_KEY_Q, GP_KEY_R, GP_KEY_S, GP_KEY_T,
	GP_KEY_U, GP_KEY_V, GP_KEY_W, GP_KEY_X, GP_KEY_Y,
	GP_KEY_Z,

	GP_KEY_1, GP_KEY_2, GP_KEY_3, GP_KEY_4, GP_KEY_5,
	GP_KEY_6, GP_KEY_7, GP_KEY_8, GP_KEY_9, GP_KEY_0,

	GP_KEY_ENTER,      GP_KEY_ESC,        GP_KEY_BACKSPACE,
	GP_KEY_TAB,        GP_KEY_SPACE,      GP_KEY_MINUS,
	GP_KEY_EQUAL,      GP_KEY_LEFT_BRACE, GP_KEY_RIGHT_BRACE,
	GP_KEY_BACKSLASH,  GP_KEY_BACKSLASH,  GP_KEY_SEMICOLON,
	GP_KEY_APOSTROPHE, GP_KEY_GRAVE,      GP_KEY_COMMA,
	GP_KEY_DOT,        GP_KEY_SLASH,      GP_KEY_CAPS_LOCK,

	GP_KEY_F1, GP_KEY_F2, GP_KEY_F3, GP_KEY_F4, GP_KEY_F5, GP_KEY_F6,
	GP_KEY_F7, GP_KEY_F8, GP_KEY_F9, GP_KEY_F10, GP_KEY_F11, GP_KEY_F12,

	GP_KEY_SYSRQ,  GP_KEY_SCROLL_LOCK, GP_KEY_PAUSE,
	GP_KEY_INSERT, GP_KEY_HOME,        GP_KEY_PAGE_UP,
	GP_KEY_DELETE, GP_KEY_END,         GP_KEY_PAGE_DOWN,

	GP_KEY_RIGHT, GP_KEY_LEFT, GP_KEY_DOWN, GP_KEY_UP,

	GP_KEY_NUM_LOCK, GP_KEY_KP_SLASH, GP_KEY_KP_ASTERISK,
	GP_KEY_KP_MINUS, GP_KEY_KP_PLUS,  GP_KEY_KP_ENTER,
	GP_KEY_KP_1,     GP_KEY_KP_2,     GP_KEY_KP_3,
	GP_KEY_KP_4,     GP_KEY_KP_5,     GP_KEY_KP_6,
	GP_KEY_KP_7,     GP_KEY_KP_8,     GP_KEY_KP_9,
	GP_KEY_KP_0,     GP_KEY_KP_DOT,

	GP_KEY_102ND, GP_KEY_COMPOSE, GP_KEY_POWER, GP_KEY_KP_EQUAL,

	GP_KEY_F13, GP_KEY_F14, GP_KEY_F15, GP_KEY_F16, GP_KEY_F17, GP_KEY_F18,
	GP_KEY_F19, GP_KEY_F20, GP_KEY_F21, GP_KEY_F22, GP_KEY_F23, GP_KEY_F24,

	GP_KEY_OPEN,      GP_KEY_HELP,        GP_KEY_PROPS,
	GP_KEY_FRONT,     GP_KEY_STOP,        GP_KEY_AGAIN,
	GP_KEY_UNDO,      GP_KEY_CUT,         GP_KEY_COPY,
	GP_KEY_PASTE,     GP_KEY_FIND,        GP_KEY_MUTE,
	GP_KEY_VOLUME_UP, GP_KEY_VOLUME_DOWN,

	/* Keyboard locking caps/num/scroll lock */
	0, 0, 0,

	GP_KEY_KP_COMMA, GP_KEY_KP_EQUAL,

	/* Keyboard international 1-9 */
	GP_KEY_RO,     GP_KEY_KATAKANA_HIRAGANA, GP_KEY_YEN,
	GP_KEY_HENKAN, GP_KEY_MUHENKAN,          GP_KEY_KP_JPCOMMA,
	0, 0, 0,

	/* Keyboard LANG 1-9 */
	GP_KEY_HANGEUL, GP_KEY_HANJA, GP_KEY_KATAKANA,
	GP_KEY_HIRAGANA, GP_KEY_ZENKAKUHANKAKU, 0,
	0, 0, 0,

	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,

	/* Reserved 0xa5 - 0xaf */
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,

	/* Various keypad keys  */
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,

	/* Reserved 0xde - 0xdf */
	0, 0,

	/* Control keys */
	GP_KEY_LEFT_CTRL,  GP_KEY_LEFT_SHIFT,  GP_KEY_LEFT_ALT,  GP_KEY_LEFT_META,
	GP_KEY_RIGHT_CTRL, GP_KEY_RIGHT_SHIFT, GP_KEY_RIGHT_ALT, GP_KEY_RIGHT_META,

	/* 0xe8 - 0xfb */
	GP_KEY_PLAY_PAUSE,  GP_KEY_STOP_CD,    GP_KEY_PREVIOUS_SONG,
	GP_KEY_NEXT_SONG,   GP_KEY_EJECT_CD,   GP_KEY_VOLUME_UP,
	GP_KEY_VOLUME_DOWN, GP_KEY_MUTE,       GP_KEY_WWW,
	GP_KEY_BACK,        GP_KEY_FORWARD,    GP_KEY_STOP,
	GP_KEY_FIND,        GP_KEY_SCROLL_UP,  GP_KEY_SCROLL_DOWN,
	GP_KEY_EDIT,        GP_KEY_SLEEP,      GP_KEY_COFFEE,
	GP_KEY_REFRESH,     GP_KEY_CALC
};

static int key_in_report(hid_keyboard_report_t const *report, uint8_t key)
{
	unsigned int i;

	for (i = 0; i <= 5; i++) {
		if (key = report->keycode[i])
			return 1;
	}

	return 0;
}

static void key_report(uint8_t key, int state)
{
	if (!keymap[key])
		return;

	gp_ev_queue_push_key(gp_rtos_ev_queue, keymap[key], state, 0);
}

static void handle_keyboard(hid_keyboard_report_t const *report)
{
	static hid_keyboard_report_t prev_report = {};
	unsigned int i;

	uint8_t mods = prev_report.modifier ^ report->modifier;

	for (i = 0; i < GP_ARRAY_SIZE(mod_keys); i++) {
		if (mods & mod_keys[i].mask) {
			if (report->modifier & mod_keys[i].mask)
				gp_ev_queue_push_key(gp_rtos_ev_queue, mod_keys[i].key, GP_EV_KEY_DOWN, 0);
			else
				gp_ev_queue_push_key(gp_rtos_ev_queue, mod_keys[i].key, GP_EV_KEY_UP, 0);
		}
	}

	for (i = 0; i <= 5; i++) {
		uint8_t key = prev_report.keycode[i];

		if (!key)
			break;

		if (!key_in_report(report, key))
			key_report(key, GP_EV_KEY_UP);
	}

	for (i = 0; i <= 5; i++) {
		uint8_t key = report->keycode[i];

		if (!key)
			break;

		if (!key_in_report(&prev_report, key))
			key_report(key, GP_EV_KEY_DOWN);
	}

	memcpy(&prev_report, report, sizeof(prev_report));
}

static void mouse_btn_report(uint8_t buttons_change, uint8_t buttons_prev,
                             uint8_t button_mask, uint16_t button_scancode)
{
	if (!(button_mask & buttons_change))
		return;

	if (button_mask & buttons_prev)
		gp_ev_queue_push_key(gp_rtos_ev_queue, button_scancode, GP_EV_KEY_UP, 0);
	else
		gp_ev_queue_push_key(gp_rtos_ev_queue, button_scancode, GP_EV_KEY_DOWN, 0);
}

static void handle_mouse(hid_mouse_report_t const *report)
{
	static uint8_t buttons_prev = 0;

	uint8_t buttons_change = report->buttons ^ buttons_prev;

	mouse_btn_report(buttons_change, buttons_prev, 0x01, GP_BTN_LEFT);
	mouse_btn_report(buttons_change, buttons_prev, 0x02, GP_BTN_MIDDLE);
	mouse_btn_report(buttons_change, buttons_prev, 0x04, GP_BTN_RIGHT);
	mouse_btn_report(buttons_change, buttons_prev, 0x08, GP_BTN_SIDE);
	mouse_btn_report(buttons_change, buttons_prev, 0x10, GP_BTN_EXTRA);

	if (report->x || report->y)
		gp_ev_queue_push_rel(gp_rtos_ev_queue, report->x, report->y, 0);

	if (report->wheel)
		gp_ev_queue_push_wheel(gp_rtos_ev_queue, report->wheel, 0);

	buttons_prev = report->buttons;
}

void tuh_hid_report_received_cb(uint8_t dev_addr, uint8_t instance, uint8_t const* report, uint16_t len)
{
	uint8_t protocol = tuh_hid_interface_protocol(dev_addr, instance);

	switch (protocol) {
	case HID_ITF_PROTOCOL_KEYBOARD:
		handle_keyboard((hid_keyboard_report_t const*)report);
	break;
	case HID_ITF_PROTOCOL_MOUSE:
		handle_mouse((hid_mouse_report_t const*)report);
	break;
	}

	if (!tuh_hid_receive_report(dev_addr, instance))
		GP_WARN("Cannot enable reports!");
}

void tuh_hid_umount_cb(uint8_t dev_addr, uint8_t instance)
{
	uint8_t protocol = tuh_hid_interface_protocol(dev_addr, instance);

	GP_DEBUG(1, "HID device unplugged addr=%" PRIu8 "inst=%" PRIu8, dev_addr, instance);

	switch (protocol) {
	case HID_ITF_PROTOCOL_KEYBOARD:
		kbd_feedback_unregister(dev_addr, instance);
	break;
	}
}
