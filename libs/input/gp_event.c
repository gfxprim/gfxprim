// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2009-2021 Cyril Hrubis <metan@ucw.cz>
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <inttypes.h>

#include <core/gp_debug.h>
#include <core/gp_common.h>

#include <input/gp_ev_queue.h>
#include <utils/gp_timer.h>
#include <input/gp_event.h>

#include <input/gp_keys.h>

int gp_ev_any_key_pressed_(gp_event *ev, ...)
{
	va_list ap;
	uint32_t key;

	va_start(ap, ev);

	for (;;) {
		key = va_arg(ap, uint32_t);

		if (!key) {
			va_end(ap);
			return 0;
		}

		if (gp_ev_key_pressed(ev, key)) {
			va_end(ap);
			return 1;
		}
	}
}

int gp_ev_all_keys_pressed_(gp_event *ev, ...)
{
	va_list ap;
	uint32_t key;

	va_start(ap, ev);

	for (;;) {
		key = va_arg(ap, uint32_t);

		if (!key) {
			va_end(ap);
			return 1;
		}

		if (!gp_ev_key_pressed(ev, key)) {
			va_end(ap);
			return 0;
		}
	}
}

static void dump_rel(gp_event *ev)
{
	printf("Rel ");

	switch (ev->code) {
	case GP_EV_REL_POS:
		printf("Position %u %u dx=%i dy=%i\n",
		       ev->st->cursor_x, ev->st->cursor_y,
		       ev->rel.rx, ev->rel.ry);
	break;
	case GP_EV_REL_WHEEL:
		printf("Wheel %i\n", ev->val);
	break;
	}
}

static void dump_key(gp_event *ev)
{
	const char *name = gp_ev_key_name(ev->key.key);

	printf("Key %i (Key%s) %s\n",
	       ev->key.key, name, ev->code ? "down" : "up");
}

static void dump_abs(gp_event *ev)
{
	switch (ev->code) {
	case GP_EV_ABS_POS:
		printf("Position %u %u %u\n",
		       ev->st->cursor_x, ev->st->cursor_y, ev->abs.pressure);
	break;
	}
}

static void dump_sys(gp_event *ev)
{
	switch (ev->code) {
	case GP_EV_SYS_QUIT:
		printf("Sys Quit\n");
	break;
	case GP_EV_SYS_RESIZE:
		printf("Sys Resize %ux%u\n", ev->sys.w, ev->sys.h);
	break;
	case GP_EV_SYS_CLIPBOARD:
		printf("Clipboard data ready\n");
	break;
	case GP_EV_SYS_BACKLIGHT:
		printf("Backlight brightness changed to %i%%\n", (int)ev->val);
	break;
	}
}

void gp_ev_dump(gp_event *ev)
{
	printf("Event (%"PRIu64") ", ev->time);

	switch (ev->type) {
	case GP_EV_KEY:
		dump_key(ev);
	break;
	case GP_EV_REL:
		dump_rel(ev);
	break;
	case GP_EV_ABS:
		dump_abs(ev);
	break;
	case GP_EV_SYS:
		dump_sys(ev);
	break;
	case GP_EV_TMR:
		printf("Timer %s expired\n", ev->tmr->id);
	break;
	case GP_EV_UTF:
		printf("Unicode char %04x\n", ev->utf.ch);
	break;
	default:
		printf("Unknown %u\n", ev->type);
	}
}
