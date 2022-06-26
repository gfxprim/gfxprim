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

static inline void send_keypress(gp_widget *widget, int value)
{

	gp_event ev_down = {
		.type = GP_EV_KEY,
		.code = GP_EV_KEY_DOWN,
		.key = {.key = value},
		.st = &events_state,
	};

	gp_event ev_up = {
		.type = GP_EV_KEY,
		.code = GP_EV_KEY_UP,
		.key = {.key = value},
		.st = &events_state,
	};

	gp_widget_ops_event(widget, NULL, &ev_down);
	gp_widget_ops_event(widget, NULL, &ev_up);
}

static inline void type_string(gp_widget *widget, const char *ch)
{
	while (*ch) {
		gp_event ev_utf = {
			.type = GP_EV_UTF,
			.utf = {.ch = (uint32_t)*ch},
			.st = &events_state,
		};

		gp_widget_ops_event(widget, NULL, &ev_utf);

		ch++;
	}
}
