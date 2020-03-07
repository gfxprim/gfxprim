// SPDX-License-Identifier: GPL-2.1-or-later
/*
 * Copyright (C) 2009-2013 Cyril Hrubis <metan@ucw.cz>
 */

 /*

   Simple backend timers example.

  */

#include <stdio.h>
#include <gfxprim.h>

static void redraw(gp_backend *self)
{
	gp_pixmap *pixmap = self->pixmap;
	gp_pixel black_pixel = gp_rgb_to_pixmap_pixel(0x00, 0x00, 0x00, pixmap);

	gp_fill(pixmap, black_pixel);

	/* Update the backend screen */
	gp_backend_flip(self);
}

static uint32_t timer_callback(gp_timer *self)
{
	uint32_t next = random() % 10000;

	printf("Timer %s callback called, rescheduling after %u.\n",
	       self->id, (unsigned) next);

	return next;
}

int main(void)
{
	gp_backend *backend;
	const char *backend_opts = "X11:100x100";

	backend = gp_backend_init(backend_opts, "Backend Timers Example");

	if (backend == NULL) {
		fprintf(stderr, "Failed to initialize backend\n");
		return 1;
	}

	redraw(backend);

	/*
	 * Periodic timer with 1000ms interval. As the callback is set to NULL
	 * Timer Event is pushed to event queue upon expiration.
	 */
	GP_TIMER_DECLARE(timer1, 0, 1000, "Timer 1", NULL, NULL);

	/*
	 * Timer with a callback, this timer gets scheduled depending on ouput
	 * from callback (0 means disable timer).
	 */
	GP_TIMER_DECLARE(timer2, 5000, 0, "Timer 2", timer_callback, NULL);

	gp_backend_add_timer(backend, &timer1);
	gp_backend_add_timer(backend, &timer2);

	/* Handle events */
	for (;;) {
		gp_event ev;

		gp_backend_wait_event(backend, &ev);

		gp_event_dump(&ev);

		switch (ev.type) {
		case GP_EV_KEY:
			switch (ev.val.val) {
			case GP_KEY_ESC:
			case GP_KEY_Q:
				gp_backend_exit(backend);
				return 0;
			break;
			}
		break;
		case GP_EV_SYS:
			switch (ev.code) {
			case GP_EV_SYS_RESIZE:
				gp_backend_resize_ack(backend);
				redraw(backend);
			break;
			case GP_EV_SYS_QUIT:
				gp_backend_exit(backend);
				return 0;
			break;
			}
		break;
		}
	}

	gp_backend_exit(backend);

	return 0;
}
