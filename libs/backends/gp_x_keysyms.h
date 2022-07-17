// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2009-2019 Cyril Hrubis <metan@ucw.cz>
 */

#ifndef BACKENDS_X_KEYSYMS_H
#define BACKENDS_X_KEYSYMS_H

#include <X11/keysym.h>
#include <X11/XF86keysym.h>

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
	{XF86XK_AudioLowerVolume, GP_KEY_VOLUME_DOWN},
	{XF86XK_AudioRaiseVolume, GP_KEY_VOLUME_UP},
	{XF86XK_AudioMute,        GP_KEY_MUTE},
	{XF86XK_AudioNext,        GP_KEY_NEXT_SONG},
	{XF86XK_AudioPrev,        GP_KEY_PREVIOUS_SONG},
	{XF86XK_AudioPlay,        GP_KEY_PLAY_PAUSE},
};

#endif /* BACKENDS_X_KEYSYMS_H */
