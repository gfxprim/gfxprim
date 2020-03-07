// SPDX-License-Identifier: GPL-2.1-or-later
/*
 * Copyright (C) 2009-2013 Cyril Hrubis <metan@ucw.cz>
 */

/*

  Linux input device driver.

 */

#ifndef INPUT_GP_INPUT_DRIVER_LINUX_H
#define INPUT_GP_INPUT_DRIVER_LINUX_H

#include <stdint.h>

#include <input/gp_types.h>

typedef struct gp_input_driver_linux gp_input_driver_linux;

struct gp_input_driver_linux {
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

	int abs_x_max;
	int abs_y_max;
	int abs_press_max;

	uint8_t abs_flag_x:1;
	uint8_t abs_flag_y:1;
	uint8_t abs_pen_flag:1;
};

/*
 * Initalize and allocate input driver.
 */
gp_input_driver_linux *gp_input_driver_linux_open(const char *path);

/*
 * Close the fd, free memory.
 */
void gp_input_driver_linux_close(gp_input_driver_linux *self);

/*
 * Called when there are data ready on input device.
 *
 * May or may not generate gp_event.
 *
 * Returns 0 on succes -1 on error and errno is set.
 */
int gp_input_driver_linux_read(gp_input_driver_linux *self,
                               gp_event_queue *event_queue);

#endif /* INPUT_GP_INPUT_DRIVER_LINUX_H */
