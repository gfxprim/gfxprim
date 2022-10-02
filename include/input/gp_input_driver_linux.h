// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2009-2020 Cyril Hrubis <metan@ucw.cz>
 */

/*

  Linux input device driver.

 */

#ifndef INPUT_GP_INPUT_DRIVER_LINUX_H
#define INPUT_GP_INPUT_DRIVER_LINUX_H

#include <stdint.h>

#include <input/gp_types.h>

typedef struct gp_input_linux gp_input_linux;

struct gp_input_linux {
	/* fd */
	int fd;

	/* to store rel coordinates */
	int rel_x;
	int rel_y;

	uint8_t rel_flag;

	/* to store abs coordinates */
	int abs_x;
	int abs_y;
	int abs_press;

	int abs_min_x;
	int abs_max_x;
	int abs_max_y;
	int abs_min_y;
	int abs_press_max;

	uint8_t abs_flag_x:1;
	uint8_t abs_flag_y:1;
	uint8_t abs_pen_flag:1;

	uint8_t abs_swap:1;
	uint8_t abs_mirror_x:1;
	uint8_t abs_mirror_y:1;
};

/*
 * Initalize and allocate input driver.
 */
gp_input_linux *gp_input_linux_open(const char *path);

/*
 * Scans the input device for a device with attribute name, returns on first match.
 */
gp_input_linux *gp_input_linux_by_name(const char *name);

/*
 * Opens a input device by description string.
 *
 * The string may include callibration data and other values as well.
 *
 * The format of the string is list of a key value pairs divided by a colons.
 *
 * The first pair has to describe a device to be opened which is one of:
 *  - name=device name
 *  - path=/dev/input/eventX
 *
 * Which may be followed by callibration such as:
 *
 * - abs_swap
 * - abs_mirror_x
 * - abs_mirror_y
 * - abs_min_x=VAL
 * - abs_max_x=VAL
 * - abs_min_y=VAL
 * - abs_max_y=VAL
 *
 * Example devstr:
 *
 * "path=/dev/input/event3:abs_swap:abs_mirror_x:abs_min_x=234:abs_max_x=3920:abs_min_y=322:abs_max_y=3249"
 */
gp_input_linux *gp_input_linux_by_devstr(const char *devstr);

/*
 * Close the fd, free memory.
 */
void gp_input_linux_close(gp_input_linux *self);

/*
 * Called when there are data ready on input device.
 *
 * May or may not generate gp_event.
 *
 * Returns 0 on succes -1 on error and errno is set.
 */
int gp_input_linux_read(gp_input_linux *self,
                        gp_ev_queue *event_queue);

#endif /* INPUT_GP_INPUT_DRIVER_LINUX_H */
