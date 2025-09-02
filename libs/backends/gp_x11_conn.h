// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2009-2024 Cyril Hrubis <metan@ucw.cz>
 */

/*
 * X11 connection, singleton.
 */
struct x11_conn {
	Display *dpy;

	/* X Atoms */
	Atom A_WM_DELETE_WINDOW;

	/* NetWM Atoms */
	Atom A__NET_WM_STATE;
	Atom A__NET_WM_STATE_FULLSCREEN;

	/* Clipboard Atoms */
	Atom A_CLIPBOARD;
	Atom A_TARGETS;
	Atom A_UTF8_STRING;
	Atom A_STRING;
	Atom A_TEXT;
	Atom A_ATOM;
	Atom A_XSEL_DATA;

	/* Bitflags for supported Atoms */
	int S__NET_WM_STATE:1;
	int S__NET_WM_STATE_FULLSCREEN:1;

	/* Is set to 1 if connection is local -> we can use SHM */
	int local:1;

	Cursor cursor_arrow;
	Cursor cursor_text_edit;
	Cursor cursor_crosshair;
	Cursor cursor_hand;
	Cursor cursor_empty;
	Cursor cursor_saved;

	/* reference counter, incremented on window creation */
	unsigned int ref_cnt;
};

static struct x11_conn x11_conn = {
	.dpy = NULL,
	.ref_cnt = 0,
};

static int x11_get_property(Atom type, Atom **args, unsigned long *count)
{
	int ret, format;
	unsigned long bytesafter;

	ret = XGetWindowProperty(x11_conn.dpy, XDefaultRootWindow(x11_conn.dpy),
	                         type, 0, 16384, False, AnyPropertyType, &type,
				 &format, count, &bytesafter, (void*)args);

	return ret == Success && *count > 0;
}

#define ATOM_SUPPORTED(name, type, atom) do {                        \
	if (x11_conn.A_##name == atom) {                             \
		GP_DEBUG(2, type " Atom '" #name "' is supported."); \
		x11_conn.S_##name = 1;                               \
	}                                                            \
} while (0)

static void x11_check_atoms(Atom at)
{
	ATOM_SUPPORTED(_NET_WM_STATE, "NetWM", at);
	ATOM_SUPPORTED(_NET_WM_STATE_FULLSCREEN, "NetWM", at);
}

#define INIT_ATOM(name) x11_conn.A_##name = XInternAtom(x11_conn.dpy, #name, False)

static void x11_detect_wm_features(void)
{
	Atom *args = NULL, at;
	unsigned long count, i;

	INIT_ATOM(WM_DELETE_WINDOW);
	INIT_ATOM(_NET_WM_STATE);
	INIT_ATOM(_NET_WM_STATE_FULLSCREEN);

	INIT_ATOM(CLIPBOARD);
	INIT_ATOM(TARGETS);
	INIT_ATOM(TEXT);
	INIT_ATOM(STRING);
	INIT_ATOM(UTF8_STRING);
	INIT_ATOM(ATOM);
	INIT_ATOM(XSEL_DATA);

	at = XInternAtom(x11_conn.dpy, "_NET_SUPPORTED", True);

	if (x11_get_property(at, &args, &count)) {
		GP_DEBUG(1, "Window manager supports NetWM");

		for (i = 0; i < count; i++)
			x11_check_atoms(args[i]);

		XFree(args);
	}
}

static void create_empty_cursor(void)
{

	Pixmap empty_bitmap;
	XColor black = {.red = 0, .blue = 0, .green = 0};
	static char empty_pixels[] = {0, 0, 0, 0, 0, 0, 0, 0};
	Window root_win = XDefaultRootWindow(x11_conn.dpy);

	empty_bitmap = XCreateBitmapFromData(x11_conn.dpy, root_win, empty_pixels, 8, 8);
	x11_conn.cursor_empty = XCreatePixmapCursor(x11_conn.dpy,
	                                            empty_bitmap, empty_bitmap,
	                                            &black, &black, 0, 0);
}

static void x11_get_cursors(void)
{
	x11_conn.cursor_arrow = XCreateFontCursor(x11_conn.dpy, XC_left_ptr);
	x11_conn.cursor_text_edit = XCreateFontCursor(x11_conn.dpy, XC_xterm);
	x11_conn.cursor_crosshair = XCreateFontCursor(x11_conn.dpy, XC_tcross);
	x11_conn.cursor_hand = XCreateFontCursor(x11_conn.dpy, XC_hand2);

	x11_conn.cursor_saved = x11_conn.cursor_arrow;

	create_empty_cursor();
}

static void x11_input_init(void);

static unsigned int x11_open(const char *display)
{
	const char *disp;

	if (x11_conn.ref_cnt != 0)
		return ++x11_conn.ref_cnt;

	GP_DEBUG(1, "Opening X11 display '%s'", display);

	disp = XDisplayName(display);

	if (disp && (disp[0] == ':' || (disp[0] == '0' && disp[1] == ':')))
		x11_conn.local = 1;
	else
		x11_conn.local = 0;

	GP_DEBUG(1, "Connection is %s (%s)",
	         x11_conn.local ? "local" : "remote", disp);

	if (!XInitThreads()) {
		GP_DEBUG(2, "XInitThreads failed");
		return 0;
	}

	x11_conn.dpy = XOpenDisplay(display);

	if (x11_conn.dpy == NULL) {
		GP_WARN("Failed to initialize X11 display");
		return 0;
	}

	/* Initialized key translation table */
	x11_input_init();
	x11_detect_wm_features();
	x11_get_cursors();

	return ++x11_conn.ref_cnt;
}

static void x11_close(void)
{
	/* Ignore close requests if connection is closed */
	if (x11_conn.ref_cnt == 0)
		return;

	if (--x11_conn.ref_cnt != 0)
		return;

	GP_DEBUG(1, "Closing X11 display");

	XLockDisplay(x11_conn.dpy);

	/* I wonder if this is right sequence... */
	//XUnlockDisplay(x11_conn.dpy);
	XCloseDisplay(x11_conn.dpy);
}
