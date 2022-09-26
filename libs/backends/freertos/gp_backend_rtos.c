// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2022 Cyril Hrubis <metan@ucw.cz>
 */

#include <core/gp_debug.h>
#include <core/gp_pixmap.h>
#include <backends/gp_backend_freertos.h>
#include "FreeRTOS.h"
#include "task.h"

static void rtos_poll(gp_backend *self)
{
	vTaskDelay(100);
}

static void rtos_wait(gp_backend *self)
{
	for (;;) {
		if (gp_event_queue_events(&self->event_queue))
			return;

		rtos_poll(self);
	}
}

static void rtos_flip(gp_backend *self)
{
	GP_DEBUG(2, "Flipping buffer");
}

static void rtos_update_rect(gp_backend *self, gp_coord x0, gp_coord y0,
                             gp_coord x1, gp_coord y1)
{
}

static gp_pixmap rtos_pixmap;

static gp_backend rtos_backend = {
	.name = "Backend RTOS",
	.pixmap = &rtos_pixmap,
	.poll = rtos_poll,
	.wait = rtos_wait,
	.flip = rtos_flip,
	.update_rect = rtos_update_rect,
};

gp_event_queue *gp_rtos_ev_queue;

gp_backend *gp_backend_rtos_init(gp_size w, gp_size h, gp_pixel pixel_type, void *fb)
{
	gp_pixmap_init(&rtos_pixmap, w, h, pixel_type, fb, 0);
	gp_event_queue_init(&rtos_backend.event_queue, w, h, 0, GP_EVENT_QUEUE_LOAD_KEYMAP);

	gp_rtos_ev_queue = &rtos_backend.event_queue;

	return &rtos_backend;
}
