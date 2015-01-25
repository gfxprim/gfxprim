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

#include <X11/keysym.h>
#include <X11/XF86keysym.h>

/* X11 keycodes */
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

/* X11 KeySyms to load dynamically */
struct keytable {
	uint32_t x_keysym;
	uint16_t key;
};

static const struct keytable sym_to_key[] = {
	/* Cursor control */
	{XK_Up,        GP_KEY_UP},
	{XK_Down,      GP_KEY_DOWN},
	{XK_Left,      GP_KEY_LEFT},
	{XK_Right,     GP_KEY_RIGHT},
	{XK_Home,      GP_KEY_HOME},
	{XK_Page_Up,   GP_KEY_PAGE_UP},
	{XK_Page_Down, GP_KEY_PAGE_DOWN},
	{XK_End,       GP_KEY_END},

	/* TTY Keys */
	{XK_BackSpace,   GP_KEY_BACKSPACE},
	{XK_Tab,         GP_KEY_TAB},
	{XK_Escape,      GP_KEY_ESC},
	{XK_Delete,      GP_KEY_DELETE},
	{XK_Insert,      GP_KEY_INSERT},
	{XK_Sys_Req,     GP_KEY_SYSRQ},
	{XK_Pause,       GP_KEY_PAUSE},
	{XK_Scroll_Lock, GP_KEY_SCROLL_LOCK},

	/* Modifiers */
	{XK_Shift_L,   GP_KEY_LEFT_SHIFT},
	{XK_Shift_R,   GP_KEY_RIGHT_SHIFT},
	{XK_Alt_L,     GP_KEY_LEFT_ALT},
	{XK_Alt_R,     GP_KEY_RIGHT_ALT},
	{XK_Control_L, GP_KEY_LEFT_CTRL},
	{XK_Control_R, GP_KEY_RIGHT_CTRL},
	{XK_Caps_Lock, GP_KEY_CAPS_LOCK},
	{XK_Super_L,   GP_KEY_LEFT_META},
	{XK_Super_R,   GP_KEY_RIGHT_META},
	{XK_Menu,      GP_KEY_COMPOSE},

	/* keypad */
	{XK_KP_Enter,    GP_KEY_KP_ENTER},
	{XK_KP_0,        GP_KEY_KP_0},
	{XK_KP_1,        GP_KEY_KP_1},
	{XK_KP_2,        GP_KEY_KP_2},
	{XK_KP_3,        GP_KEY_KP_3},
	{XK_KP_4,        GP_KEY_KP_4},
	{XK_KP_5,        GP_KEY_KP_5},
	{XK_KP_6,        GP_KEY_KP_6},
	{XK_KP_7,        GP_KEY_KP_7},
	{XK_KP_8,        GP_KEY_KP_8},
	{XK_KP_9,        GP_KEY_KP_9},
	{XK_KP_Add,      GP_KEY_KP_PLUS},
	{XK_KP_Subtract, GP_KEY_KP_MINUS},
	{XK_KP_Decimal,  GP_KEY_KP_DOT},
	{XK_KP_Divide,   GP_KEY_KP_SLASH},
	{XK_KP_Multiply, GP_KEY_KP_ASTERISK},

	/* Multimedia */
	{XF86XK_AudioLowerVolume, GP_KEY_VOLUMEDOWN},
	{XF86XK_AudioRaiseVolume, GP_KEY_VOLUMEUP},
	{XF86XK_AudioMute,        GP_KEY_MUTE},
	{XF86XK_AudioNext,        GP_KEY_NEXTSONG},
	{XF86XK_AudioPrev,        GP_KEY_PREVIOUSSONG},
	{XF86XK_AudioPlay,        GP_KEY_PLAYPAUSE},
};

static void x11_input_init(void)
{
	GP_DEBUG(1, "Initializing X11 KeyCode table");

	unsigned int i;

	for (i = 0; i < GP_ARRAY_SIZE(sym_to_key); i++) {
		unsigned int keycode;

		keycode = XKeysymToKeycode(x11_conn.dpy,
		                           sym_to_key[i].x_keysym);

		if (keycode == 0) {
			GP_DEBUG(1, "KeySym '%s' (%u) not defined",
			         XKeysymToString(sym_to_key[i].x_keysym),
			         sym_to_key[i].x_keysym);
			continue;
		}

		GP_DEBUG(3, "Mapping Key '%s' KeySym '%s' (%u) to KeyCode %u",
		         GP_EventKeyName(sym_to_key[i].key),
				 XKeysymToString(sym_to_key[i].x_keysym),
				 sym_to_key[i].x_keysym,
				 keycode);

		if (keycode - 9 >= GP_ARRAY_SIZE(keycode_table)) {
			GP_WARN("Key '%s' keycode %u out of table",
			        GP_EventKeyName(sym_to_key[i].key), keycode);
			continue;
		}

		if (keycode_table[keycode - 9]) {
			GP_WARN("Key '%s' keycode %u collides with key '%s'",
			        GP_EventKeyName(sym_to_key[i].key), keycode,
					GP_EventKeyName(keycode_table[keycode - 9]));
			continue;
		}

		keycode_table[keycode - 9] = sym_to_key[i].key;
	}
}

static unsigned int get_key(unsigned int xkey)
{
	unsigned int key = 0;

	if (xkey > 8 && xkey - 9 <= GP_ARRAY_SIZE(keycode_table))
		key = keycode_table[xkey - 9];

	if (key == 0)
		GP_WARN("Unmapped X11 keycode 0x%02x %u", xkey, xkey);

	return key;
}

static void x11_input_event_put(struct GP_EventQueue *event_queue,
                                XEvent *ev, int w, int h)
{
	int key = 0, press = 0;

	switch (ev->type) {
	case ButtonPress:
		press = 1;
	case ButtonRelease:
		switch (ev->xbutton.button) {
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
				GP_EventQueuePush(event_queue, GP_EV_REL,
				                  GP_EV_REL_WHEEL, 1, NULL);
			return;
		case 5:
			if (press)
				GP_EventQueuePush(event_queue, GP_EV_REL,
				                  GP_EV_REL_WHEEL, -1, NULL);
			return;
		}

		if (key == 0) {
			GP_WARN("Unmapped X11 button %02x",
			        ev->xbutton.button);
			return;
		}

		GP_EventQueuePush(event_queue, GP_EV_KEY, key, press, NULL);
	break;
	case ConfigureNotify:
		GP_EventQueuePushResize(event_queue, ev->xconfigure.width,
		                        ev->xconfigure.height, NULL);
	break;
	break;
	case MotionNotify:
		/* Ignore all pointer events that are out of the window */
		if (ev->xmotion.x < 0 || ev->xmotion.y < 0 ||
		    ev->xmotion.x > w || ev->xmotion.y > h)
			return;

		GP_EventQueuePushRelTo(event_queue,
		                       ev->xmotion.x, ev->xmotion.y, NULL);
	break;
	case KeyPress:
		press = 1;
	case KeyRelease:
		key = get_key(ev->xkey.keycode);

		if (key == 0)
			return;

		GP_EventQueuePushKey(event_queue, key, press, NULL);
	break;
	/* events from WM */
	case ClientMessage:
		if ((Atom)ev->xclient.data.l[0] == x11_conn.A_WM_DELETE_WINDOW) {
			GP_EventQueuePush(event_queue, GP_EV_SYS,
			                  GP_EV_SYS_QUIT, 0, NULL);
			return;
		}
		GP_WARN("Unknown X Client Message");
	break;
	case MapNotify:
		GP_DEBUG(1, "MapNotify event received");
	break;
	case ReparentNotify:
		GP_DEBUG(1, "ReparentNotify event received");
	break;
	case GravityNotify:
		GP_DEBUG(1, "GravityNotify event received");
	break;
	case UnmapNotify:
		GP_DEBUG(1, "UnmapNotify event received");
	break;
	default:
		GP_WARN("Unhandled X11 event type %u", ev->type);
	}
}
