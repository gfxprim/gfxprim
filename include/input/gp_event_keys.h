// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2009-2022 Cyril Hrubis <metan@ucw.cz>
 */

/**
 * @file gp_event_keys.h
 * @brief Key event values.
 */

#ifndef INPUT_GP_EVENT_KEYS_H
#define INPUT_GP_EVENT_KEYS_H

/**
 * @brief A key event values.
 *
 * This is 1:1 to linux kernel input subsystem.
 *
 * @image html keyboard.svg
 */
enum gp_event_key_value {
	/** @brief Key escape */
	GP_KEY_ESC            =  1,
	/** @brief Key 1 */
	GP_KEY_1              =  2,
	/** @brief Key 2 */
	GP_KEY_2              =  3,
	/** @brief Key 3 */
	GP_KEY_3              =  4,
	/** @brief Key 4 */
	GP_KEY_4              =  5,
	/** @brief Key 5 */
	GP_KEY_5              =  6,
	/** @brief Key 6 */
	GP_KEY_6              =  7,
	/** @brief Key 7 */
	GP_KEY_7              =  8,
	/** @brief Key 8 */
	GP_KEY_8              =  9,
	/** @brief Key 9 */
	GP_KEY_9              = 10,
	/** @brief Key 0 */
	GP_KEY_0              = 11,
	/** @brief Key minus */
	GP_KEY_MINUS          = 12,
	/** @brief Key equal */
	GP_KEY_EQUAL          = 13,
	/** @brief Key backspace */
	GP_KEY_BACKSPACE      = 14,
	/** @brief Key tab */
	GP_KEY_TAB            = 15,
	/** @brief Key Q */
	GP_KEY_Q              = 16,
	/** @brief Key W */
	GP_KEY_W              = 17,
	/** @brief Key E */
	GP_KEY_E              = 18,
	/** @brief Key R */
	GP_KEY_R              = 19,
	/** @brief Key T */
	GP_KEY_T              = 20,
	/** @brief Key Y */
	GP_KEY_Y              = 21,
	/** @brief Key U */
	GP_KEY_U              = 22,
	/** @brief Key I */
	GP_KEY_I              = 23,
	/** @brief Key O */
	GP_KEY_O              = 24,
	/** @brief Key P */
	GP_KEY_P              = 25,
	/** @brief Key Left Brace */
	GP_KEY_LEFT_BRACE     = 26,
	/** @brief Key Right Brace */
	GP_KEY_RIGHT_BRACE    = 27,
	/** @brief Enter */
	GP_KEY_ENTER          = 28,
	/** @brief Key Left Ctrl */
	GP_KEY_LEFT_CTRL      = 29,
	/** @brief Key A */
	GP_KEY_A              = 30,
	/** @brief Key S */
	GP_KEY_S              = 31,
	/** @brief Key D */
	GP_KEY_D              = 32,
	/** @brief Key F */
	GP_KEY_F              = 33,
	/** @brief Key G */
	GP_KEY_G              = 34,
	/** @brief Key H */
	GP_KEY_H              = 35,
	/** @brief Key J */
	GP_KEY_J              = 36,
	/** @brief Key K */
	GP_KEY_K              = 37,
	/** @brief Key L */
	GP_KEY_L              = 38,
	/** @brief Key Semicolon */
	GP_KEY_SEMICOLON      = 39,
	/** @brief Key Apostrophe */
	GP_KEY_APOSTROPHE     = 40,
	/** @brief Key Grave */
	GP_KEY_GRAVE          = 41,
	/** @brief Key Left Shift */
	GP_KEY_LEFT_SHIFT     = 42,
	/** @brief Key Back Slash */
	GP_KEY_BACKSLASH      = 43,
	/** @brief Key Z */
	GP_KEY_Z              = 44,
	/** @brief Key X */
	GP_KEY_X              = 45,
	/** @brief Key C */
	GP_KEY_C              = 46,
	/** @brief Key V */
	GP_KEY_V              = 47,
	/** @brief Key B */
	GP_KEY_B              = 48,
	/** @brief Key N */
	GP_KEY_N              = 49,
	/** @brief Key M */
	GP_KEY_M              = 50,
	/** @brief Key Comma */
	GP_KEY_COMMA          = 51,
	/** @brief Key Dot */
	GP_KEY_DOT            = 52,
	/** @brief Key Slash */
	GP_KEY_SLASH          = 53,
	/** @brief Key Right Shift */
	GP_KEY_RIGHT_SHIFT    = 54,
	/** @brief Key Asterisk */
	GP_KEY_KP_ASTERISK    = 55,
	/** @brief Key Left Alt */
	GP_KEY_LEFT_ALT       = 56,
	/** @brief Key Space */
	GP_KEY_SPACE          = 57,
	/** @brief Key Caps Lock */
	GP_KEY_CAPS_LOCK      = 58,
	/** @brief Key F1 */
	GP_KEY_F1             = 59,
	/** @brief Key F2 */
	GP_KEY_F2             = 60,
	/** @brief Key F3 */
	GP_KEY_F3             = 61,
	/** @brief Key F4 */
	GP_KEY_F4             = 62,
	/** @brief Key F5 */
	GP_KEY_F5             = 63,
	/** @brief Key F6 */
	GP_KEY_F6             = 64,
	/** @brief Key F7 */
	GP_KEY_F7             = 65,
	/** @brief Key F8 */
	GP_KEY_F8             = 66,
	/** @brief Key F9 */
	GP_KEY_F9             = 67,
	/** @brief Key F10 */
	GP_KEY_F10            = 68,
	/** @brief Key Num Lock */
	GP_KEY_NUM_LOCK       = 69,
	/** @brief Key Scroll Lock */
	GP_KEY_SCROLL_LOCK    = 70,
	/** @brief Key Keypad 7 */
	GP_KEY_KP_7           = 71,
	/** @brief Key Keypad 8 */
	GP_KEY_KP_8           = 72,
	/** @brief Key Keypad 9 */
	GP_KEY_KP_9           = 73,
	/** @brief Key Keypad Minus */
	GP_KEY_KP_MINUS       = 74,
	/** @brief Key Keypad 4 */
	GP_KEY_KP_4           = 75,
	/** @brief Key Keypad 5 */
	GP_KEY_KP_5           = 76,
	/** @brief Key Keypad 6 */
	GP_KEY_KP_6           = 77,
	/** @brief Key Keypad Plus */
	GP_KEY_KP_PLUS        = 78,
	/** @brief Key Keypad 1 */
	GP_KEY_KP_1           = 79,
	/** @brief Key Keypad 2 */
	GP_KEY_KP_2           = 80,
	/** @brief Key Keypad 3 */
	GP_KEY_KP_3           = 81,
	/** @brief Key Keypad 0 */
	GP_KEY_KP_0           = 82,
	/** @brief Key Keypad Dot */
	GP_KEY_KP_DOT         = 83,

	GP_KEY_ZENKAKUHANKAKU = 85,
	GP_KEY_102ND          = 86,
	/** @brief Key F11 */
	GP_KEY_F11            = 87,
	/** @brief Key F12 */
	GP_KEY_F12            = 88,
	GP_KEY_RO             = 89,
	GP_KEY_KATAKANA       = 90,
	GP_KEY_HIRAGANA       = 91,
	GP_KEY_HENKAN         = 92,
	GP_KEY_KATAKANA_HIRAGANA = 93,
	GP_KEY_MUHENKAN       = 94,
	GP_KEY_KP_JPCOMMA     = 95,
	GP_KEY_KP_ENTER       = 96,
	/** @brief Key Right Ctrl */
	GP_KEY_RIGHT_CTRL     = 97,
	/** @brief Key Keypad Slash */
	GP_KEY_KP_SLASH       = 98,
	/** @brief Key System Request */
	GP_KEY_SYSRQ          = 99,
	/** @brief Key Right Alt */
	GP_KEY_RIGHT_ALT      = 100,
	GP_KEY_LINEFEED       = 101,
	/** @brief Key Home */
	GP_KEY_HOME           = 102,
	/** @brief Key Up */
	GP_KEY_UP             = 103,
	/** @brief Key Page Up */
	GP_KEY_PAGE_UP        = 104,
	/** @brief Key Left */
	GP_KEY_LEFT           = 105,
	/** @brief Key Right */
	GP_KEY_RIGHT          = 106,
	/** @brief Key End */
	GP_KEY_END            = 107,
	/** @brief Key Down */
	GP_KEY_DOWN           = 108,
	/** @brief Key Page Down */
	GP_KEY_PAGE_DOWN      = 109,
	/** @brief Key Insert */
	GP_KEY_INSERT         = 110,
	/** @brief Key Delete */
	GP_KEY_DELETE         = 111,
	GP_KEY_MACRO          = 112,
	GP_KEY_MUTE           = 113,
	GP_KEY_VOLUME_DOWN    = 114,
	GP_KEY_VOLUME_UP      = 115,
	GP_KEY_POWER          = 116,
	/** @brief Key Keypad Equal */
	GP_KEY_KP_EQUAL       = 117,
	/** @brief Key Keypad Plus Minus */
	GP_KEY_KP_PLUS_MINUS  = 118,
	/** @brief Key Pause */
	GP_KEY_PAUSE          = 119,
	GP_KEY_SCALE          = 120,
	/** @brief Key Keypad Comma */
	GP_KEY_KP_COMMA       = 121,
	GP_KEY_HANGEUL        = 122,
	GP_KEY_HANJA          = 123,
	GP_KEY_YEN            = 124,
	/** @brief Key Left Meta */
	GP_KEY_LEFT_META      = 125,
	/** @brief Key Right Meta */
	GP_KEY_RIGHT_META     = 126,
	GP_KEY_COMPOSE        = 127,
	GP_KEY_STOP           = 128,
	GP_KEY_AGAIN          = 129,
	GP_KEY_PROPS          = 130,
	GP_KEY_UNDO           = 131,
	GP_KEY_FRONT          = 132,
	GP_KEY_COPY           = 133,
	GP_KEY_OPEN           = 134,
	GP_KEY_PASTE          = 135,
	GP_KEY_FIND           = 136,
	GP_KEY_CUT            = 137,
	GP_KEY_HELP           = 138,
	GP_KEY_MENU           = 139,
	GP_KEY_CALC           = 140,
	GP_KEY_SETUP          = 141,
	GP_KEY_SLEEP          = 142,
	GP_KEY_WAKEUP         = 143,
	GP_KEY_FILE           = 144,
	GP_KEY_SEND_FILE      = 145,
	GP_KEY_DELETE_FILE    = 146,
	GP_KEY_XFER           = 147,
	GP_KEY_PROG1          = 148,
	GP_KEY_PROG2          = 149,
	GP_KEY_WWW            = 150,
	GP_KEY_MSDOS          = 151,
	GP_KEY_COFFEE         = 152,
	GP_KEY_ROTATE_DISPLAY = 153,
	GP_KEY_CYCLE_WINDOWS  = 154,
	GP_KEY_MAIL           = 155,
	GP_KEY_BOOKMARKS      = 156,
	GP_KEY_COMPUTER       = 157,
	GP_KEY_BACK           = 158,
	GP_KEY_FORWARD        = 159,
	GP_KEY_CLOSE_CD       = 160,
	GP_KEY_EJECT_CD       = 161,
	GP_KEY_EJECT_CLOSE_CD  = 162,
	GP_KEY_NEXT_SONG      = 163,
	GP_KEY_PLAY_PAUSE     = 164,
	GP_KEY_PREVIOUS_SONG  = 165,
	GP_KEY_STOP_CD        = 166,
	GP_KEY_RECORD         = 167,
	GP_KEY_REWIND         = 168,
	GP_KEY_PHONE          = 169,
	GP_KEY_ISO            = 170,
	GP_KEY_CONFIG         = 171,
	GP_KEY_HOME_PAGE      = 172,
	GP_KEY_REFRESH        = 173,
	GP_KEY_EXIT           = 174,
	GP_KEY_MOVE           = 175,
	GP_KEY_EDIT           = 176,
	GP_KEY_SCROLL_UP      = 177,
	GP_KEY_SCROLL_DOWN    = 178,
	GP_KEY_KP_LEFT_PAREN  = 179,
	GP_KEY_KP_RIGHT_PAREN = 180,
	GP_KEY_NEW            = 181,
	GP_KEY_REDO           = 182,
	GP_KEY_F13            = 183,
	GP_KEY_F14            = 184,
	GP_KEY_F15            = 185,
	GP_KEY_F16            = 186,
	GP_KEY_F17            = 187,
	GP_KEY_F18            = 188,
	GP_KEY_F19            = 189,
	GP_KEY_F20            = 190,
	GP_KEY_F21            = 191,
	GP_KEY_F22            = 192,
	GP_KEY_F23            = 193,
	GP_KEY_F24            = 194,

	GP_KEY_PLAY_CD        = 200,
	GP_KEY_PAUSE_CD       = 201,
	GP_KEY_PROG3          = 202,
	GP_KEY_PROG4          = 203,
	GP_KEY_DASHBOARD      = 204,
	GP_KEY_SUSPEND        = 205,
	GP_KEY_CLOSE          = 206,
	GP_KEY_PLAY           = 207,
	GP_KEY_FASTFORWARD    = 208,
	GP_KEY_BASS_BOOST     = 209,
	GP_KEY_PRINT          = 210,
	GP_KEY_HP             = 211,
	GP_KEY_CAMERA         = 212,
	GP_KEY_SOUND          = 213,
	GP_KEY_QUESTION       = 214,
	GP_KEY_EMAIL          = 215,
	GP_KEY_CHAT           = 216,
	GP_KEY_SEARCH         = 217,
	GP_KEY_CONNECT        = 218,
	GP_KEY_FINANCE        = 219,
	GP_KEY_SPORT          = 220,
	GP_KEY_SHOP           = 221,
	GP_KEY_ALT_ERASE      = 222,
	GP_KEY_CANCEL         = 223,
	GP_KEY_BRIGHTNESS_DOWN = 224,
	GP_KEY_BRIGHTNESS_UP  = 225,
	GP_KEY_MEDIA          = 226,

	GP_KEY_SWITCH_VIDEO_MODE = 227,

	GP_KEY_KBD_ILLUM_TOGGLE = 228,
	GP_KEY_KBD_ILLUM_DOWN = 229,
	GP_KEY_KBD_ILLUM_UP   = 230,

	GP_KEY_SEND           = 231,
	GP_KEY_REPLY          = 232,
	GP_KEY_FORWARD_MAIL   = 233,
	GP_KEY_SAVE           = 234,
	GP_KEY_DOCUMENTS      = 235,

	GP_KEY_BATTERY        = 236,

	GP_KEY_BLUETOOTH      = 237,
	GP_KEY_WLAN           = 238,
	GP_KEY_UWB            = 239,

	GP_KEY_UNKNOWN        = 240,

	GP_KEY_VIDEO_NEXT     = 241,
	GP_KEY_VIDEO_PREV     = 242,
	GP_KEY_BRIGHTNESS_CYCLE = 243,
	GP_KEY_BRIGHTNESS_AUTO  = 244,

	GP_KEY_DISPLAY_OFF    = 245,

	GP_KEY_WWAN           = 246,
	GP_KEY_RFKILL         = 247,

	GP_KEY_MIC_MUTE       = 248,

	/* Common Buttons */
	GP_BTN_0              = 0x100,
	GP_BTN_1              = 0x101,
	GP_BTN_2              = 0x102,
	GP_BTN_3              = 0x103,
	GP_BTN_4              = 0x104,
	GP_BTN_5              = 0x105,
	GP_BTN_6              = 0x106,
	GP_BTN_7              = 0x107,
	GP_BTN_8              = 0x108,
	GP_BTN_9              = 0x109,

	/** @brief Left Mouse Button */
	GP_BTN_LEFT           = 0x110,
	/** @brief Right Mouse Button */
	GP_BTN_RIGHT          = 0x111,
	/** @brief Middle Mouse Button */
	GP_BTN_MIDDLE         = 0x112,
	/** @brief Side Mouse Button */
	GP_BTN_SIDE           = 0x113,
	/** @brief Extra Mouse Button */
	GP_BTN_EXTRA          = 0x114,
	/** @brief Forward Mouse Button */
	GP_BTN_FORWARD        = 0x115,
	/** @brief Back Mouse Button */
	GP_BTN_BACK           = 0x116,
	GP_BTN_TASK           = 0x117,

	GP_BTN_JOYSTICK       = 0x120,
	GP_BTN_THUMB          = 0x121,
	GP_BTN_THUMB2         = 0x122,
	GP_BTN_TOP            = 0x123,
	GP_BTN_TOP2           = 0x124,
	GP_BTN_PINKIE         = 0x125,
	GP_BTN_BASE           = 0x126,
	GP_BTN_BASE2          = 0x127,
	GP_BTN_BASE3          = 0x128,
	GP_BTN_BASE4          = 0x129,
	GP_BTN_BASE5          = 0x12a,
	GP_BTN_BASE6          = 0x12b,
	GP_BTN_DEAD           = 0x12f,

	GP_BTN_SOUTH          = 0x130,
	GP_BTN_EAST           = 0x131,
	GP_BTN_C              = 0x132,
	GP_BTN_NORTH          = 0x133,
	GP_BTN_WEST           = 0x134,
	GP_BTN_Z              = 0x135,
	GP_BTN_TL             = 0x136,
	GP_BTN_TR             = 0x137,
	GP_BTN_TL2            = 0x138,
	GP_BTN_TR2            = 0x139,
	GP_BTN_SELECT         = 0x13a,
	GP_BTN_START          = 0x13b,
	GP_BTN_MODE           = 0x13c,
	GP_BTN_THUMBL         = 0x13d,
	GP_BTN_THUMBR         = 0x13e,

	GP_BTN_TOOL_PEN       = 0x140,
	GP_BTN_TOOL_RUBBER    = 0x141,
	GP_BTN_TOOL_BRUSH     = 0x142,
	GP_BTN_TOOL_PENCIL    = 0x143,
	GP_BTN_TOOL_AIRBRUSH  = 0x144,
	GP_BTN_TOOL_FINGER    = 0x145,
	GP_BTN_TOOL_MOUSE     = 0x146,
	GP_BTN_TOOL_LENS      = 0x147,
	GP_BTN_TOOL_QUINT_TAP = 0x148,
	GP_BTN_STYLUS3        = 0x149,
	/**
	 * @brief A touch screen touched event.
	 *
	 * Formerly GP_BTN_PEN.
	 */
	GP_BTN_TOUCH          = 0x14a,
	GP_BTN_STYLUS         = 0x14b,
	GP_BTN_STYLUS2        = 0x14c,
	GP_BTN_TOOL_DOUBLE_TAP = 0x14d,
	GP_BTN_TOOL_TRIPLE_TAP = 0x14e,
	GP_BTN_TOOL_QUAD_TAP  = 0x14f,

	/** @brief Mouse Wheel Button */
	GP_BTN_WHEEL          = 0x150,
	GP_BTN_GEAR_UP        = 0x151,

	GP_KEY_OK             = 0x160,
	GP_KEY_SELECT         = 0x161,
	GP_KEY_GOTO           = 0x162,
	GP_KEY_CLEAR          = 0x163,
	GP_KEY_POWER2         = 0x164,
	GP_KEY_OPTION         = 0x165,
	GP_KEY_INFO           = 0x166,
	GP_KEY_TIME           = 0x167,
	GP_KEY_VENDOR         = 0x168,
	GP_KEY_ARCHIVE        = 0x169,
	GP_KEY_PROGRAM        = 0x16a,
	GP_KEY_CHANNEL        = 0x16b,
	GP_KEY_FAVORITES      = 0x16c,
	GP_KEY_EPG            = 0x16d,
	GP_KEY_PVR            = 0x16e,
	GP_KEY_MHP            = 0x16f,
	GP_KEY_LANGUAGE       = 0x170,
	GP_KEY_TITLE          = 0x171,
	GP_KEY_SUBTITLE       = 0x172,
	GP_KEY_ANGLE          = 0x173,
	GP_KEY_FULL_SCREEN    = 0x174,
	GP_KEY_MODE           = 0x175,
	GP_KEY_KEYBOARD       = 0x176,
	GP_KEY_ASPECT_RATIO   = 0x177,
	GP_KEY_PC             = 0x178,
	GP_KEY_TV             = 0x179,
	GP_KEY_TV2            = 0x17a,
	GP_KEY_VCR            = 0x17b,
	GP_KEY_VCR2           = 0x17c,
	GP_KEY_SAT            = 0x17d,
	GP_KEY_SAT2           = 0x17e,
	GP_KEY_CD             = 0x17f,
	GP_KEY_TAPE           = 0x180,
	GP_KEY_RADIO          = 0x181,
	GP_KEY_TUNER          = 0x182,
	GP_KEY_PLAYER         = 0x183,
	GP_KEY_TEXT           = 0x184,
	GP_KEY_DVD            = 0x185,
	GP_KEY_AUX            = 0x186,
	GP_KEY_MP3            = 0x187,
	GP_KEY_AUDIO          = 0x188,
	GP_KEY_VIDEO          = 0x189,
	GP_KEY_DIRECTORY      = 0x18a,
	GP_KEY_LIST           = 0x18b,
	GP_KEY_MEMO           = 0x18c,
	GP_KEY_CALENDAR       = 0x18d,
	GP_KEY_RED            = 0x18e,
	GP_KEY_GREEN          = 0x18f,
	GP_KEY_YELLOW         = 0x190,
	GP_KEY_BLUE           = 0x191,
	GP_KEY_CHANNEL_UP     = 0x192,
	GP_KEY_CHANNEL_DOWN   = 0x193,
	GP_KEY_FIRST          = 0x194,
	GP_KEY_LAST           = 0x195,
	GP_KEY_AB             = 0x196,
	GP_KEY_NEXT           = 0x197,
	GP_KEY_RESTART        = 0x198,
	GP_KEY_SLOW           = 0x199,
	GP_KEY_SHUFFLE        = 0x19a,
	GP_KEY_BREAK          = 0x19b,
	GP_KEY_PREVIOUS       = 0x19c,
	GP_KEY_DIGITS         = 0x19d,
	GP_KEY_TEEN           = 0x19e,
	GP_KEY_TWEN           = 0x19f,
	GP_KEY_VIDEO_PHONE    = 0x1a0,
	GP_KEY_GAMES          = 0x1a1,
	GP_KEY_ZOOM_IN        = 0x1a2,
	GP_KEY_ZOOM_OUT       = 0x1a3,
	GP_KEY_ZOOM_RESET     = 0x1a4,
	GP_KEY_WORD_PROCESSOR  = 0x1a5,
	GP_KEY_EDITOR         = 0x1a6,
	GP_KEY_SPREADSHEET    = 0x1a7,
	GP_KEY_GRAPHICS_EDITOR = 0x1a8,
	GP_KEY_PRESENTATION   = 0x1a9,
	GP_KEY_DATABASE       = 0x1aa,
	GP_KEY_NEWS           = 0x1ab,
	GP_KEY_VOICE_MAIL     = 0x1ac,
	GP_KEY_ADDRESS_BOOK   = 0x1ad,
	GP_KEY_MESSENGER      = 0x1ae,
	GP_KEY_DISPLAY_TOGGLE = 0x1af,
	GP_KEY_SPELL_CHECK    = 0x1b0,
	GP_KEY_LOGOFF         = 0x1b1,

	GP_KEY_DOLLAR         = 0x1b2,
	GP_KEY_EURO           = 0x1b3,

	GP_KEY_FRAME_BACK     = 0x1b4,
	GP_KEY_FRAME_FORWARD  = 0x1b5,
	GP_KEY_CONTEXT_MENU   = 0x1b6,
	GP_KEY_MEDIA_REPEAT   = 0x1b7,
	GP_KEY_10CHANNELS_UP  = 0x1b8,
	GP_KEY_10CHANNELS_DOWN = 0x1b9,
	GP_KEY_IMAGES         = 0x1ba,
	GP_KEY_NOTIFICATION_CENTER = 0x1bc,
	GP_KEY_PICKUP_PHONE   = 0x1bd,
	GP_KEY_HANGUP_PHONE   = 0x1be,

	GP_KEY_DEL_EOL        = 0x1c0,
	GP_KEY_DEL_EOS        = 0x1c1,
	GP_KEY_INS_LINE       = 0x1c2,
	GP_KEY_DEL_LINE       = 0x1c3,
};

#endif /* INPUT_GP_EVENT_KEYS_H */
