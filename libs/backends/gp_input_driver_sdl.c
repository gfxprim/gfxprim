// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2009-2013 Cyril Hrubis <metan@ucw.cz>
 */

#include "../../config.h"

#ifdef HAVE_LIBSDL

#if LIBSDL_VERSION == 1
# include <SDL/SDL.h>
#elif LIBSDL_VERSION == 2
# include <SDL2/SDL.h>
#endif

#include "core/gp_common.h"
#include <core/gp_debug.h>

#include <input/gp_ev_queue.h>
#include "gp_input_driver_sdl.h"

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

#if LIBSDL_VERSION == 1
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
#else
/* SDL2 has scancodes instead */
static uint16_t keysym_scancodes[] = {
	[57] = GP_KEY_CAPS_LOCK,
	[58] = GP_KEY_F1,
	[59] = GP_KEY_F2,
	[60] = GP_KEY_F3,
	[61] = GP_KEY_F4,
	[62] = GP_KEY_F5,
	[63] = GP_KEY_F6,
	[64] = GP_KEY_F7,
	[65] = GP_KEY_F8,
	[66] = GP_KEY_F9,
	[67] = GP_KEY_F10,
	[68] = GP_KEY_F11,
	[69] = GP_KEY_F12,
	[70] = GP_KEY_SYSRQ,

	[73] = GP_KEY_INSERT,
	[74] = GP_KEY_HOME,
	[75] = GP_KEY_PAGE_UP,

	[77] = GP_KEY_END,
	[78] = GP_KEY_PAGE_DOWN,
	[79] = GP_KEY_RIGHT,
	[80] = GP_KEY_LEFT,
	[81] = GP_KEY_DOWN,
	[82] = GP_KEY_UP,

	[224] = GP_KEY_LEFT_CTRL,
	[225] = GP_KEY_LEFT_SHIFT,
	[226] = GP_KEY_LEFT_ALT,
	[227] = GP_KEY_LEFT_META,
	[228] = GP_KEY_RIGHT_CTRL,
	[229] = GP_KEY_RIGHT_SHIFT,
	[230] = GP_KEY_RIGHT_ALT,
};
#endif

void gp_input_driver_sdl_event_put(gp_backend *backend, gp_ev_queue *event_queue, SDL_Event *ev)
{
	uint32_t keysym;
	uint32_t key = 0;

	switch (ev->type) {
	case SDL_MOUSEMOTION:
		gp_ev_queue_push_rel_to(event_queue, ev->motion.x,
		                        ev->motion.y, 0);
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
		/* Mouse wheel */
		case 4:
			if (ev->type == SDL_MOUSEBUTTONDOWN)
				gp_ev_queue_push(event_queue, GP_EV_REL,
				                    GP_EV_REL_WHEEL, 1, 0);
			return;
		case 5:
			if (ev->type == SDL_MOUSEBUTTONDOWN)
				gp_ev_queue_push(event_queue, GP_EV_REL,
				                    GP_EV_REL_WHEEL, -1, 0);
			return;
		default:
			GP_WARN("Unmapped SDL Mouse button %02x",
			         ev->button.button);
			return;
		}

		gp_ev_queue_push(event_queue, GP_EV_KEY,
		                  key, ev->button.state, 0);
	break;
	case SDL_KEYDOWN:
	case SDL_KEYUP:
		keysym = ev->key.keysym.sym;

#if LIBSDL_VERSION == 1
		if (keysym > 0 && keysym <= GP_ARRAY_SIZE(keysym_table1))
			key = keysym_table1[keysym - 1];

		if (keysym > 255 && keysym <= 255 + GP_ARRAY_SIZE(keysym_table2))
			key = keysym_table2[keysym - 256];

		if (key == 0) {
			GP_WARN("Unmapped SDL keysym %u", keysym);
			return;
		}

#else
		if (keysym & SDLK_SCANCODE_MASK) {
			keysym &= ~SDLK_SCANCODE_MASK;

			if (keysym < GP_ARRAY_SIZE(keysym_scancodes))
				key = keysym_scancodes[keysym];

			if (!key) {
				GP_WARN("Unmapped SDL2 scancode %u", keysym);
				return;
			}
		} else {
			if (key < GP_ARRAY_SIZE(keysym_table1))
				key = keysym_table1[keysym - 1];

			if (!key) {
				GP_WARN("Unmapped SDL2 keycode %u", keysym);
				return;
			}
		}
#endif

		gp_ev_queue_push_key(event_queue, key, ev->key.state,
#if LIBSDL_VERSION == 1
				ev->key.keysym.unicode,
#else
				0, //TODO Unicode handling in libsdl2.0?
#endif
				0);

#if LIBSDL_VERSION == 1
		//KEYMAP CTRL
		if (ev->key.keysym.unicode)
			gp_ev_queue_push_utf(event_queue, ev->key.keysym.unicode, 0);
#endif
	break;
#if LIBSDL_VERSION == 1
	case SDL_VIDEORESIZE:
		gp_ev_queue_push_resize(event_queue, ev->resize.w,
		                           ev->resize.h, 0);
	break;
	case SDL_VIDEOEXPOSE:
		gp_backend_flip(backend);
	break;
#elif LIBSDL_VERSION == 2
	case SDL_WINDOWEVENT:
		switch (ev->window.event) {
		case SDL_WINDOWEVENT_SIZE_CHANGED:
		case SDL_WINDOWEVENT_RESIZED:
			gp_ev_queue_push_resize(event_queue, ev->window.data1,
			                        ev->window.data2, 0);
		break;
		case SDL_WINDOWEVENT_EXPOSED:
			gp_backend_flip(backend);
		break;
		case SDL_WINDOWEVENT_FOCUS_GAINED:
			gp_ev_queue_push(event_queue, GP_EV_SYS,
			                 GP_EV_SYS_FOCUS, GP_EV_SYS_FOCUS_IN, 0);
		break;
		case SDL_WINDOWEVENT_FOCUS_LOST:
			gp_ev_queue_push(event_queue, GP_EV_SYS,
			                 GP_EV_SYS_FOCUS, GP_EV_SYS_FOCUS_OUT, 0);
		break;
		case SDL_WINDOWEVENT_SHOWN:
			gp_ev_queue_push(event_queue, GP_EV_SYS,
			                 GP_EV_SYS_VISIBILITY, GP_EV_SYS_VISIBILITY_SHOW, 0);
		break;
		case SDL_WINDOWEVENT_HIDDEN:
			gp_ev_queue_push(event_queue, GP_EV_SYS,
			                 GP_EV_SYS_VISIBILITY, GP_EV_SYS_VISIBILITY_HIDE, 0);
		break;
		default:
			GP_DEBUG(0, "Unhandled event %i\n", ev->window.event);
		}
	break;
#endif
	case SDL_QUIT:
		gp_ev_queue_push(event_queue, GP_EV_SYS,
		                  GP_EV_SYS_QUIT, 0, 0);
	break;
	}
}

#endif /* HAVE_LIBSDL */
