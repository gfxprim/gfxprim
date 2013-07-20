/*****************************************************************************
 * This file is part of gfxprim library.                                     *
 *                                                                           *
 * Gfxprim is free software; you can redistribute it and/or                  *
 * modify it under the terms of the GNU Lesser General Public                *
 * License as published by the Free Software Foundation; either              *
 * version 2.1 of the License, or (at your option) any later version.        *
 *                                                                           *
 * Gfxprim is distributed in the hope that it will be useful,                *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of            *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU         *
 * Lesser General Public License for more details.                           *
 *                                                                           *
 * You should have received a copy of the GNU Lesser General Public          *
 * License along with gfxprim; if not, write to the Free Software            *
 * Foundation, Inc., 51 Franklin Street, Fifth Floor,                        *
 * Boston, MA  02110-1301  USA                                               *
 *                                                                           *
 * Copyright (C) 2009-2013 Cyril Hrubis <metan@ucw.cz>                       *
 *                                                                           *
 *****************************************************************************/

 /*

   Simple backend timers example.

  */

#include <GP.h>

static void redraw(GP_Backend *self)
{
	GP_Context *context = self->context;
	GP_Pixel black_pixel = GP_ColorToContextPixel(GP_COL_BLACK, context);

	GP_Fill(context, black_pixel);

	/* Update the backend screen */
	GP_BackendFlip(self);
}

static uint32_t timer_callback(GP_Timer *self)
{
	uint32_t next = random() % 10000;

	printf("Timer %s callback called, rescheduling after %u.\n",
	       self->id, (unsigned) next);

	return next;
}

int main(void)
{
	GP_Backend *backend;
	const char *backend_opts = "X11:100x100";

	backend = GP_BackendInit(backend_opts, "Backend Timers Example", stderr);

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

	GP_BackendAddTimer(backend, &timer1);
	GP_BackendAddTimer(backend, &timer2);

	/* Handle events */
	for (;;) {
		GP_Event ev;

		GP_BackendWaitEvent(backend, &ev);

		GP_EventDump(&ev);

		switch (ev.type) {
		case GP_EV_KEY:
			switch (ev.val.val) {
			case GP_KEY_ESC:
			case GP_KEY_Q:
				GP_BackendExit(backend);
				return 0;
			break;
			}
		break;
		case GP_EV_SYS:
			switch (ev.code) {
			case GP_EV_SYS_RESIZE:
				GP_BackendResizeAck(backend);
				redraw(backend);
			break;
			case GP_EV_SYS_QUIT:
				GP_BackendExit(backend);
				return 0;
			break;
			}
		break;
		}
	}

	GP_BackendExit(backend);

	return 0;
}
