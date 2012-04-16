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

#ifdef HAVE_LIBSDL

#include "GP_Event.h"
#include "GP_InputDriverSDL.h"

/* SDL ascii mapped keys */
static uint16_t keysym_table1[] = {
 0,                  0,                 0,                 0,
 0,                  0,                 0,                 GP_KEY_BACKSPACE,
 GP_KEY_TAB,         0,                 0,                 0,
 GP_KEY_ENTER,       0,                 0,                 0,
 0,                  0,                 GP_KEY_PAUSE,      0,
 0,                  0,                 0,                 0,
 0,                  0,                 GP_KEY_ESC,        0,
 0,                  0,                 0,                 GP_KEY_SPACE,
 0,                  0,                 0,                 0,
 0,                  0,                 GP_KEY_APOSTROPHE, 0,
 0,                  0,                 0,                 GP_KEY_COMMA,
 GP_KEY_MINUS,       GP_KEY_DOT,        GP_KEY_SLASH,      GP_KEY_0,
 GP_KEY_1,           GP_KEY_2,          GP_KEY_3,          GP_KEY_4,
 GP_KEY_5,           GP_KEY_6,          GP_KEY_7,          GP_KEY_8,
 GP_KEY_9,           0,                 GP_KEY_SEMICOLON,  0,
 GP_KEY_EQUAL,       0,                 0,                 0,
 0,                  0,                 0,                 0,
 0,                  0,                 0,                 0,
 0,                  0,                 0,                 0,
 0,                  0,                 0,                 0,
 0,                  0,                 0,                 0,
 0,                  0,                 0,                 0,
 0,                  0,                 GP_KEY_LEFT_BRACE, GP_KEY_BACKSLASH,
 GP_KEY_RIGHT_BRACE, 0,                 0,                 GP_KEY_GRAVE,
 GP_KEY_A,           GP_KEY_B,          GP_KEY_C,          GP_KEY_D,
 GP_KEY_E,           GP_KEY_F,          GP_KEY_G,          GP_KEY_H,
 GP_KEY_I,           GP_KEY_J,          GP_KEY_K,          GP_KEY_L,
 GP_KEY_M,           GP_KEY_N,          GP_KEY_O,          GP_KEY_P,
 GP_KEY_Q,           GP_KEY_R,          GP_KEY_S,          GP_KEY_T,
 GP_KEY_U,           GP_KEY_V,          GP_KEY_W,          GP_KEY_X,
 GP_KEY_Y,           GP_KEY_Z,          0,                 0,
 0,                  0,                 GP_KEY_DELETE,
};

static const uint16_t keysym_table1_size = sizeof(keysym_table1)/2;

/* keypad and function keys starting at 256 */
static uint16_t keysym_table2[] = {
 GP_KEY_KP_0,        GP_KEY_KP_1,       GP_KEY_KP_2,        GP_KEY_KP_3,
 GP_KEY_KP_4,        GP_KEY_KP_5,       GP_KEY_KP_6,        GP_KEY_KP_7,
 GP_KEY_KP_8,        GP_KEY_KP_9,       GP_KEY_KP_DOT,      GP_KEY_KP_SLASH,
 GP_KEY_KP_ASTERISK, GP_KEY_KP_MINUS,   GP_KEY_KP_PLUS,     GP_KEY_KP_ENTER,
 GP_KEY_KP_EQUAL,    GP_KEY_UP,         GP_KEY_DOWN,        GP_KEY_RIGHT,
 GP_KEY_LEFT,        GP_KEY_INSERT,     GP_KEY_HOME,        GP_KEY_END,
 GP_KEY_PAGE_UP,     GP_KEY_PAGE_DOWN,  GP_KEY_F1,          GP_KEY_F2,
 GP_KEY_F3,          GP_KEY_F4,         GP_KEY_F5,          GP_KEY_F6, 
 GP_KEY_F7,          GP_KEY_F8,         GP_KEY_F9,          GP_KEY_F10, 
 GP_KEY_F11,         GP_KEY_F12,        GP_KEY_F13,         GP_KEY_F14,
 GP_KEY_F15,         0,                 0,                  0,
 GP_KEY_NUM_LOCK,    GP_KEY_CAPS_LOCK,  GP_KEY_SCROLL_LOCK, GP_KEY_RIGHT_SHIFT,
 GP_KEY_LEFT_SHIFT,  GP_KEY_RIGHT_CTRL, GP_KEY_LEFT_CTRL,   GP_KEY_RIGHT_ALT,
 GP_KEY_LEFT_ALT,    0,                 0,                  GP_KEY_LEFT_META,
 GP_KEY_RIGHT_META,  0,                 0,                  0,
 GP_KEY_SYSRQ,       0,                 0,                  GP_KEY_COMPOSE,
};

static const uint16_t keysym_table2_size = sizeof(keysym_table2)/2;

void GP_InputDriverSDLEventPut(SDL_Event *ev)
{
	uint16_t keysym;
	uint32_t key = 0;

	switch (ev->type) {
	case SDL_MOUSEMOTION:
		GP_EventPushRel(ev->motion.xrel, ev->motion.yrel, NULL);
	break;
	case SDL_MOUSEBUTTONDOWN:
	case SDL_MOUSEBUTTONUP:
		switch (ev->button.button) {
		case 1:
			key = GP_BTN_LEFT;
		break;
		case 2:
			key = GP_BTN_MIDDLE;
		break;
		case 3:
			key = GP_BTN_RIGHT;
		break;
		default:
			return;
		}

		GP_EventPush(GP_EV_KEY, key, ev->button.state, NULL);
	break;
	case SDL_KEYDOWN:
	case SDL_KEYUP:
		keysym = ev->key.keysym.sym;

		if (keysym > 0 && keysym <= keysym_table1_size)
			key = keysym_table1[keysym - 1];

		if (keysym > 255 && keysym <= 255 + keysym_table2_size)
			key = keysym_table2[keysym - 256];

		if (key == 0) {
			fprintf(stderr, "Unmapped SDL keysym %u\n", keysym);
			return;
		}
		
		GP_EventPushKey(key, ev->key.state, NULL);
	break;
	case SDL_VIDEORESIZE:
	break;
	case SDL_QUIT:
	break;
	}
}

#endif /* HAVE_LIBSDL */
