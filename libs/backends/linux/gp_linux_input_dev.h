// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2009-2025 Cyril Hrubis <metan@ucw.cz>
 */

#ifndef GP_LINUX_INPUT_DEV_H
#define GP_LINUX_INPUT_DEV_H

#include <linux/input.h>
#include <backends/gp_linux_input.h>

enum gp_linux_input_device_type {
	LINUX_INPUT_NONE = 0x00,
	LINUX_INPUT_MOUSE = 0x01,
	LINUX_INPUT_TOUCHPAD = 0x02,
	LINUX_INPUT_TOUCHSCREEN = 0x03,
	LINUX_INPUT_TABLET = 0x04,
	LINUX_INPUT_JOYSTICK = 0x05,
};

/** @brief Touchpad converts abs events into relative */
struct input_touchpad {
	int abs_x;
	int abs_y;
	int last_x;
	int last_y;

	uint8_t x_valid:1;
	uint8_t y_valid:1;
	uint8_t single_tap;
	uint8_t double_tap;
	uint8_t triple_tap;
};

struct linux_input {
	gp_backend_input input;
	gp_backend *backend;

	/* Callbacks for leds e.g. Caps Lock */
	gp_ev_feedback feedback;
	uint32_t leds;

	enum gp_linux_input_device_type device_type;

	gp_fd fd;

	/* to store rel coordinates */
	int rel_x;
	int rel_y;
	int rel_wheel;

	uint8_t rel_flag;

	union {
		struct {
			/* to store abs coordinates */
			int abs_x;
			int abs_y;
			int abs_press;

			/* Coordinate limits after transformation */
			int abs_x_max;
			int abs_y_max;
			int abs_press_min;
			int abs_press_max;

			/*
			 * Affine transformation matrix
			 *
			 * The multipliers are in 16.16 fixed point format
			 */
			int abs_x_off;
			int abs_y_off;
			int abs_x_mul_x;
			int abs_x_mul_y;
			int abs_y_mul_x;
			int abs_y_mul_y;

			uint8_t abs_flag_x:1;
			uint8_t abs_flag_y:1;
			uint8_t abs_pen_flag:1;
		};
		struct input_touchpad tp;
	};
};

/**
 * @brief Processes touchpad input events.
 *
 * Touchpad is an absolute multitouch device that is converted into relative
 * input device, scrolling etc.
 *
 * @self An input device with LINUX_INPUT_TOUCHPAD device type.
 * @ev An absolute input event to be processed.
 */
void input_touchpad(struct linux_input *self, struct input_event *ev);

#endif /* GP_LINUX_INPUT_DEV_H */
