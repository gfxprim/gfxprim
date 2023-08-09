// SPDX-License-Identifier: GPL-2.1-or-later
/*
 * Copyright (C) 2009-2023 Cyril Hrubis <metan@ucw.cz>
 */

 /*

   Simple backend timers example.

  */

#include <stdio.h>
#include <gfxprim.h>

static gp_pixel black_pixel, white_pixel;
static unsigned int timer1_expirations = 0;
static unsigned int timer2_expirations = 0;

static uint32_t timer_callback(gp_timer *self)
{
	uint32_t next = random() % 10000;

	printf("Timer %s callback called, rescheduling after %u.\n",
	       self->id, (unsigned) next);

	timer2_expirations++;

	return next;
}

static void redraw(gp_backend *backend)
{
	gp_fill(backend->pixmap, black_pixel);

	gp_print(backend->pixmap, NULL, 10, 10,
		 GP_ALIGN_RIGHT | GP_VALIGN_BELOW,
		 white_pixel, black_pixel, "Timer 1 expirations %u",
	         timer1_expirations);

	gp_print(backend->pixmap, NULL, 10, 30,
		 GP_ALIGN_RIGHT | GP_VALIGN_BELOW,
		 white_pixel, black_pixel, "Timer 2 expirations %u",
	         timer2_expirations);

	gp_backend_flip(backend);
}

int main(void)
{
	gp_backend *backend;
	const char *backend_opts = NULL;

	backend = gp_backend_init(backend_opts, 200, 100, "Backend Timers Example");

	if (!backend) {
		fprintf(stderr, "Failed to initialize backend\n");
		return 1;
	}

	black_pixel = gp_rgb_to_pixmap_pixel(0x00, 0x00, 0x00, backend->pixmap);
	white_pixel = gp_rgb_to_pixmap_pixel(0xff, 0xff, 0xff, backend->pixmap);

	gp_fill(backend->pixmap, black_pixel);
	gp_backend_flip(backend);

	/*
	 * Periodic timer with 1000ms interval. As the callback is set to NULL
	 * Timer Event is pushed to event queue upon expiration. The backend
	 * timer callback returns the .period value, which reschedulles the
	 * time. You can also set .period to GP_TIMER_STOP which would make the
	 * timer oneshot.
	 *
	 * NOTE: The .expires is set to 0 which means that the timer first
	 *       expires as soon as we enter the backend wait call.
	 */
	gp_timer timer1 = {
		.expires = 0,
		.period = 1000,
		.id = "Timer 1",
	};

	/*
	 * Timer with a callback, this timer gets scheduled depending on ouput
	 * from callback (GP_TIMER_STOP) means disable timer).
	 */
	gp_timer timer2 = {
		.expires = 5000,
		.id = "Timer 2",
		.callback = timer_callback,
	};

	gp_backend_add_timer(backend, &timer1);
	gp_backend_add_timer(backend, &timer2);

	/* Handle events */
	for (;;) {
		gp_event *ev = gp_backend_wait_event(backend);

		gp_ev_dump(ev);

		switch (ev->type) {
		case GP_EV_KEY:
			switch (ev->val) {
			case GP_KEY_ESC:
			case GP_KEY_Q:
				gp_backend_exit(backend);
				return 0;
			break;
			}
		break;
		case GP_EV_SYS:
			switch (ev->code) {
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
		case GP_EV_TMR:
			timer1_expirations++;
			redraw(backend);
		break;
		}
	}

	gp_backend_exit(backend);

	return 0;
}
