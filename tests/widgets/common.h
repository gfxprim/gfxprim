// SPDX-License-Identifier: GPL-2.1-or-later
/*
 * Copyright (C) 2020-2021 Cyril Hrubis <metan@ucw.cz>
 */

static gp_events_state events_state = {};

static inline void state_press(int key)
{
	gp_events_state_press(&events_state, key);
}

static inline void state_release(int key)
{
	gp_events_state_release(&events_state, key);
}

static inline void send_keypress(gp_widget *widget, int value, char ascii)
{

	gp_event ev_down = {
		.type = GP_EV_KEY,
		.code = GP_EV_KEY_DOWN,
		.key = {.key = value, .ascii = ascii},
		.st = &events_state,
	};

	gp_event ev_up = {
		.type = GP_EV_KEY,
		.code = GP_EV_KEY_UP,
		.key = {.key = value, .ascii = ascii},
		.st = &events_state,
	};

	gp_widget_ops_event(widget, NULL, &ev_down);
	gp_widget_ops_event(widget, NULL, &ev_up);
}

static int ascii_to_keys[] = {
	GP_KEY_A, GP_KEY_B, GP_KEY_C, GP_KEY_D, GP_KEY_E, GP_KEY_F, GP_KEY_G,
	GP_KEY_H, GP_KEY_I, GP_KEY_K, GP_KEY_L, GP_KEY_M, GP_KEY_N, GP_KEY_O,
	GP_KEY_P, GP_KEY_Q, GP_KEY_R, GP_KEY_S, GP_KEY_T, GP_KEY_U, GP_KEY_V,
	GP_KEY_W, GP_KEY_X, GP_KEY_Y, GP_KEY_Z,
};

static int ascii_to_key(char ch)
{
	if (ch == ' ')
		return GP_KEY_SPACE;

	if (ch < 'a' || ch > 'z')
		return -1;

	return ascii_to_keys[ch - 'a'];
}

static inline void type_string(gp_widget *widget, const char *ch)
{
	while (*ch) {
		int key = ascii_to_key(*ch);

		if (key < 0)
			continue;

		tst_msg("Sending keypress %i (%c)", key, *ch);

		send_keypress(widget, key, *ch);
		ch++;
	}
}
