// SPDX-License-Identifier: GPL-2.1-or-later
/*
 * Copyright (C) 2009-2013 Cyril Hrubis <metan@ucw.cz>
 */

 /*

   Bare Linux Input event example.

  */

#include <gfxprim.h>

#include <input/gp_input_driver_linux.h>

int main(int argc, char *argv[])
{
	struct gp_input_driver_linux *drv;
	GP_EVENT_QUEUE_DECLARE(event_queue, 640, 480);

	gp_set_debug_level(2);

	if (argc != 2) {
		printf("Usage: %s /dev/input/eventX\n", argv[0]);
		return 1;
	}

	drv = gp_input_driver_linux_open(argv[1]);

	if (drv == NULL) {
		printf("Failed to open input device\n");
		return 1;
	}

	for (;;) {
		while (gp_input_driver_linux_read(drv, &event_queue) >= 1);

		gp_event ev;

		while (gp_event_queue_get(&event_queue, &ev))
			gp_event_dump(&ev);

		usleep(1000);
	}

	return 0;
}
