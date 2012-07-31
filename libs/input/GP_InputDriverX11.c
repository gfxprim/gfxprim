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
 * Copyright (C) 2009-2012 Cyril Hrubis <metan@ucw.cz>                       *
 *                                                                           *
 *****************************************************************************/

#include "../../config.h"

#include "core/GP_Debug.h"
#include "GP_Event.h"

#ifdef HAVE_LIBX11

#include "GP_InputDriverX11.h"

/* X11 keycodes */
static uint16_t keycode_table[] = {
 GP_KEY_ESC,         GP_KEY_1,           GP_KEY_2,           GP_KEY_3,
 GP_KEY_4,           GP_KEY_5,           GP_KEY_6,           GP_KEY_7,
 GP_KEY_8,           GP_KEY_9,           GP_KEY_0,           GP_KEY_MINUS,
 GP_KEY_EQUAL,       GP_KEY_BACKSPACE,   GP_KEY_TAB,         GP_KEY_Q,
 GP_KEY_W,           GP_KEY_E,           GP_KEY_R,           GP_KEY_T,
 GP_KEY_Y,           GP_KEY_U,           GP_KEY_I,           GP_KEY_O,
 GP_KEY_P,           GP_KEY_LEFT_BRACE,  GP_KEY_RIGHT_BRACE, GP_KEY_ENTER,
 GP_KEY_LEFT_CTRL,   GP_KEY_A,           GP_KEY_S,           GP_KEY_D,
 GP_KEY_F,           GP_KEY_G,           GP_KEY_H,           GP_KEY_J,
 GP_KEY_K,           GP_KEY_L,           GP_KEY_SEMICOLON,   GP_KEY_APOSTROPHE,
 0,                  GP_KEY_LEFT_SHIFT,  GP_KEY_BACKSLASH,   GP_KEY_Z,
 GP_KEY_X,           GP_KEY_C,           GP_KEY_V,           GP_KEY_B,
 GP_KEY_N,           GP_KEY_M,           GP_KEY_COMMA,       GP_KEY_DOT,
 GP_KEY_SLASH,       GP_KEY_RIGHT_SHIFT, 0,                  GP_KEY_LEFT_ALT,
 GP_KEY_SPACE,       GP_KEY_CAPS_LOCK,   GP_KEY_F1,          GP_KEY_F2,
 GP_KEY_F3,          GP_KEY_F4,          GP_KEY_F5,          GP_KEY_F6,
 GP_KEY_F7,          GP_KEY_F8,          GP_KEY_F9,          GP_KEY_F10,
 0,                  0,                  0,                  0,
 0,                  0,                  0,                  0,
 0,                  0,                  0,                  0,
 0,                  0,                  0,                  0,
 0,                  0,                  GP_KEY_F11,         GP_KEY_F12,
 0,                  0,                  0,                  0,
 0,                  0,                  0,                  0,
 GP_KEY_RIGHT_CTRL,  0,                  /* PRINTSCREEN */0, GP_KEY_RIGHT_ALT,
 0,                  GP_KEY_HOME,        GP_KEY_UP,          GP_KEY_PAGE_UP,
 GP_KEY_LEFT,        GP_KEY_RIGHT,       GP_KEY_END,         GP_KEY_DOWN,
 GP_KEY_PAGE_DOWN,   0,                  GP_KEY_DELETE,
};

static const uint16_t keycode_table_size = sizeof(keycode_table)/2;

void GP_InputDriverX11EventPut(XEvent *ev)
{
	int key = 0, keycode, press = 0;

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
				GP_EventPush(GP_EV_REL, GP_EV_REL_WHEEL, 1, NULL);
			return;
		case 5:
			if (press)
				GP_EventPush(GP_EV_REL, GP_EV_REL_WHEEL, -1, NULL);
			return;
		}

		if (key == 0) {
			GP_DEBUG(0, "Unmapped X11 button %02x",
			         ev->xbutton.button);
			return;
		}

		GP_EventPush(GP_EV_KEY, key, press, NULL);
	break;
	case ConfigureNotify:
		GP_EventPushResize(ev->xconfigure.width,
		                   ev->xconfigure.height, NULL);
	break;
	break;
	case MotionNotify:
		GP_EventPushRelTo(ev->xmotion.x, ev->xmotion.y, NULL);
	break;
	case KeyPress:
		press = 1;
	case KeyRelease:
		keycode = ev->xkey.keycode;

		if (keycode > 8 && keycode - 9 <= keycode_table_size)
			key = keycode_table[keycode - 9];

		if (key == 0) {
			GP_DEBUG(0, "Unmapped X11 keycode %02x", keycode);
			return;
		}
		
		GP_EventPushKey(key, press, NULL);
	break;
	default:
		GP_DEBUG(0, "Unhandled X11 event type %u", ev->type);
	}
}

#endif /* HAVE_LIBX11 */
