// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2009-2019 Cyril Hrubis <metan@ucw.cz>
 */

#include <xcb/xcb_keysyms.h>

/* XCB keycodes */
static uint16_t keycode_table[] = {
 0,                  GP_KEY_1,           GP_KEY_2,           GP_KEY_3,
 GP_KEY_4,           GP_KEY_5,           GP_KEY_6,           GP_KEY_7,
 GP_KEY_8,           GP_KEY_9,           GP_KEY_0,           GP_KEY_MINUS,
 GP_KEY_EQUAL,       0,                  0,                  GP_KEY_Q,
 GP_KEY_W,           GP_KEY_E,           GP_KEY_R,           GP_KEY_T,
 GP_KEY_Y,           GP_KEY_U,           GP_KEY_I,           GP_KEY_O,
 GP_KEY_P,           GP_KEY_LEFT_BRACE,  GP_KEY_RIGHT_BRACE, GP_KEY_ENTER,
 0,                  GP_KEY_A,           GP_KEY_S,           GP_KEY_D,
 GP_KEY_F,           GP_KEY_G,           GP_KEY_H,           GP_KEY_J,
 GP_KEY_K,           GP_KEY_L,           GP_KEY_SEMICOLON,   GP_KEY_APOSTROPHE,
 GP_KEY_GRAVE,       0,                  GP_KEY_BACKSLASH,   GP_KEY_Z,
 GP_KEY_X,           GP_KEY_C,           GP_KEY_V,           GP_KEY_B,
 GP_KEY_N,           GP_KEY_M,           GP_KEY_COMMA,       GP_KEY_DOT,
 GP_KEY_SLASH,       0,                  0,                  0,
 GP_KEY_SPACE,       0,                  GP_KEY_F1,          GP_KEY_F2,
 GP_KEY_F3,          GP_KEY_F4,          GP_KEY_F5,          GP_KEY_F6,
 GP_KEY_F7,          GP_KEY_F8,          GP_KEY_F9,          GP_KEY_F10,
 GP_KEY_NUM_LOCK,    0,                  0,                  0,
 0,                  0,                  0,                  0,
 0,                  0,                  0,                  0,
 0,                  0,                  0,                  0,
 0,                  0,                  GP_KEY_F11,         GP_KEY_F12,
 0,                  0,                  0,                  0,
 0,                  0,                  0,                  0,
 0,                  0,                  0,                  0,
 0,                  0,                  0,                  0,
 0,                  0,                  0,                  0,
 0,                  0,                  0,                  0,
 0,                  0,                  0,                  0,
 0,                  0,                  0,                  0,
 0,                  0,                  0,                  0,
 0,                  0,                  0,                  0,
 0,                  0,                  0,                  0,
 0,                  0,                  0,                  0,
 0,                  0,                  0,                  0,
 0,                  0,                  0,                  0,
 0,                  0,                  0,                  0,
 0,                  0,                  0,                  0,
 0,                  0,                  0,                  0,
 0,                  0,                  0,                  0,
 0,                  0,                  0,                  0,
 0,                  0,                  0,                  0,
 0,                  0,                  0,                  0,
 0,                  0,                  0,                  0,
 0,                  0,                  0,                  0,
 0,                  0,                  0,                  0,
};

#include "gp_x_keysyms.h"

static void xcb_input_init(xcb_connection_t *c)
{
	GP_DEBUG(1, "Initializing XCB KeyCode table");

	xcb_key_symbols_t *symtable = xcb_key_symbols_alloc(c);

	unsigned int i;

	for (i = 0; i < GP_ARRAY_SIZE(sym_to_key); i++) {
		xcb_keycode_t *keycodes;
		unsigned int keycode;

		keycodes = xcb_key_symbols_get_keycode(symtable, sym_to_key[i].x_keysym);
		keycode = keycodes[0];
		free(keycodes);

		if (keycode == XCB_NO_SYMBOL) {
			GP_DEBUG(1, "KeySym (%u) not defined",
			         sym_to_key[i].x_keysym);
			continue;
		}

		GP_DEBUG(3, "Mapping Key '%s' KeySym '%s' (%u) to KeyCode %u",
		         gp_ev_key_name(sym_to_key[i].key),
			// XKeysymToString(sym_to_key[i].x_keysym),
			 "TODO",
			 sym_to_key[i].x_keysym, keycode);

		if (keycode - 9 >= GP_ARRAY_SIZE(keycode_table)) {
			GP_WARN("Key '%s' keycode %u out of table",
			        gp_ev_key_name(sym_to_key[i].key), keycode);
			continue;
		}

		if (keycode_table[keycode - 9]) {
			GP_WARN("Key '%s' keycode %u collides with key '%s'",
			        gp_ev_key_name(sym_to_key[i].key), keycode,
				gp_ev_key_name(keycode_table[keycode - 9]));
			continue;
		}

		keycode_table[keycode - 9] = sym_to_key[i].key;
	}

	xcb_key_symbols_free(symtable);
}

static unsigned int get_key(unsigned int xkey)
{
	unsigned int key = 0;

	if (xkey > 8 && xkey - 9 <= GP_ARRAY_SIZE(keycode_table))
		key = keycode_table[xkey - 9];

	if (key == 0)
		GP_WARN("Unmapped X keycode 0x%02x %u", xkey, xkey);

	return key;
}

static void handle_visibility(gp_ev_queue *event_queue, struct win *win, int val)
{
	if (val == GP_EV_SYS_VISIBILITY_SHOW) {
		if (win->visible)
			return;
		win->visible = 1;
	}

	if (val == GP_EV_SYS_VISIBILITY_HIDE) {
		if (!win->visible)
			return;
		win->visible = 0;
	}

	gp_ev_queue_push(event_queue, GP_EV_SYS, GP_EV_SYS_VISIBILITY, val, 0);
}

static void handle_focus(gp_ev_queue *event_queue, struct win *win, int val)
{
	if (val == GP_EV_SYS_FOCUS_IN) {
		if (win->focused)
			return;
		win->focused = 1;
	}

	if (val == GP_EV_SYS_FOCUS_OUT) {
		if (!win->focused)
			return;
		win->focused = 0;
	}

	gp_ev_queue_push(event_queue, GP_EV_SYS, GP_EV_SYS_FOCUS, val, 0);
}

#ifdef HAVE_XCB_UTIL_ERRORS
# include <xcb/xcb_errors.h>
#endif

static void xcb_input_event_put(gp_ev_queue *event_queue, struct win *win,
                                xcb_generic_event_t *ev, int w, int h)
{
	int key = 0, press = 0;
	int ev_type = ev->response_type & ~0x80;

	switch (ev_type) {
#ifdef HAVE_XCB_UTIL_ERRORS
	case 0: {
		xcb_generic_error_t *err = (xcb_generic_error_t*)ev;
		xcb_errors_context_t *err_ctx;
		xcb_errors_context_new(x_con.c, &err_ctx);
		const char *major, *minor, *extension, *error;
		major = xcb_errors_get_name_for_major_code(err_ctx, err->major_code);
		minor = xcb_errors_get_name_for_minor_code(err_ctx, err->major_code, err->minor_code);
		error = xcb_errors_get_name_for_error(err_ctx, err->error_code, &extension);
		GP_WARN("XCB Error: %s:%s, %s:%s, resource %u sequence %u",
			error, extension ? extension : "no_extension",
			major, minor ? minor : "no_minor",
			(unsigned int)err->resource_id,
			(unsigned int)err->sequence);
		xcb_errors_context_free(err_ctx);
	} break;
#endif
	case XCB_BUTTON_PRESS:
		press = 1; /* fallthru */
	case XCB_BUTTON_RELEASE: {
		xcb_button_press_event_t *bev = (xcb_button_press_event_t *)ev;

		switch (bev->detail) {
		case 1:
			key = GP_BTN_LEFT;
		break;
		case 2:
			key = GP_BTN_MIDDLE;
		break;
		case 3:
			key = GP_BTN_RIGHT;
		break;
		/* Mouse wheel */
		case 4:
			if (press)
				gp_ev_queue_push(event_queue, GP_EV_REL,
				                    GP_EV_REL_WHEEL, 1, 0);
			return;
		case 5:
			if (press)
				gp_ev_queue_push(event_queue, GP_EV_REL,
				                  GP_EV_REL_WHEEL, -1, 0);
			return;
		}

		if (key == 0) {
			GP_WARN("Unmapped X button %02x", bev->detail);
			return;
		}

		gp_ev_queue_push(event_queue, GP_EV_KEY, key, press, 0);
	} break;
	case XCB_CONFIGURE_NOTIFY: {
		xcb_configure_notify_event_t *cev = (xcb_configure_notify_event_t *)ev;
		gp_ev_queue_push_resize(event_queue, cev->width, cev->height, 0);
	}
	break;
	break;
	case XCB_MOTION_NOTIFY: {
		xcb_motion_notify_event_t *mev = (xcb_motion_notify_event_t *)ev;

		/* Ignore all pointer events that are out of the window */
		if (mev->event_x < 0 || mev->event_y < 0 ||
		    mev->event_x > w || mev->event_y > h)
			return;

		gp_ev_queue_push_rel_to(event_queue,
		                           mev->event_x, mev->event_y, 0);
	}
	break;
	case XCB_KEY_PRESS:
		press = 1; /* fallthru */
	case XCB_KEY_RELEASE: {
		xcb_key_press_event_t *kev = (xcb_key_press_event_t *)ev;
		key = get_key(kev->detail);

		if (key == 0)
			return;

		gp_ev_queue_push_key(event_queue, key, press, 0);
	} break;
	/* events from WM */
	case XCB_CLIENT_MESSAGE: {
		xcb_client_message_event_t *cev = (xcb_client_message_event_t*)ev;
		if (cev->data.data32[0] == x_con.wm_delete_window) {
			gp_ev_queue_push(event_queue, GP_EV_SYS,
			                  GP_EV_SYS_QUIT, 0, 0);
			return;
		}
		GP_WARN("Unknown X Client Message");
	} break;
	case XCB_MAP_NOTIFY:
		GP_DEBUG(1, "MapNotify event received");
		handle_visibility(event_queue, win, GP_EV_SYS_VISIBILITY_SHOW);
	break;
	case XCB_REPARENT_NOTIFY:
		GP_DEBUG(1, "ReparentNotify event received");
	break;
	case XCB_GRAVITY_NOTIFY:
		GP_DEBUG(1, "GravityNotify event received");
	break;
	case XCB_UNMAP_NOTIFY:
		GP_DEBUG(1, "UnmapNotify event received");
		handle_visibility(event_queue, win, GP_EV_SYS_VISIBILITY_HIDE);
	break;
	case XCB_NO_EXPOSURE:
		GP_DEBUG(1, "NoExposure event received");
	break;
	case XCB_FOCUS_OUT:
		GP_DEBUG(1, "FocusOut event releasing all keys in events_state");
		gp_events_state_release_all(&event_queue->state);
		handle_focus(event_queue, win, GP_EV_SYS_FOCUS_OUT);
	break;
	case XCB_FOCUS_IN:
		GP_DEBUG(1, "FocusIn event received");
		handle_focus(event_queue, win, GP_EV_SYS_FOCUS_IN);
	break;
	case XCB_VISIBILITY_NOTIFY: {
		xcb_visibility_notify_event_t *cev = (xcb_visibility_notify_event_t*)ev;
		switch (cev->state) {
		case XCB_VISIBILITY_FULLY_OBSCURED:
			GP_DEBUG(1, "VisibilityFullyObscured received");
			handle_visibility(event_queue, win, GP_EV_SYS_VISIBILITY_HIDE);
		break;
		case XCB_VISIBILITY_PARTIALLY_OBSCURED:
			GP_DEBUG(1, "VisibilityPartiallyObscured received");
			handle_visibility(event_queue, win, GP_EV_SYS_VISIBILITY_SHOW);
		break;
		case XCB_VISIBILITY_UNOBSCURED:
			GP_DEBUG(1, "VisibilityUnobscured received");
			handle_visibility(event_queue, win, GP_EV_SYS_VISIBILITY_SHOW);
		break;
		}
	}
	break;
	default:
		if (ev_type == x_con.shm_completion_ev) {
			xcb_shm_completion_event_t *sev = (xcb_shm_completion_event_t*)ev;
			GP_DEBUG(1, "SHM completion shmseg=%u", (unsigned int)sev->shmseg);
			return;
		}

		GP_WARN("Unhandled X event type %u", ev->response_type & ~0x80);
	}
}
