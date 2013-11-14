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
 * X11 connection, singleton.
 */
struct x11_conn {
	Display *dpy;

	/* X Atoms */
	Atom A_WM_DELETE_WINDOW;

	/* NetWM Atoms */
	Atom A__NET_WM_STATE;
	Atom A__NET_WM_STATE_FULLSCREEN;

	/* Bitflags for supported Atoms */
	int S__NET_WM_STATE:1;
	int S__NET_WM_STATE_FULLSCREEN:1;

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

	at = XInternAtom(x11_conn.dpy, "_NET_SUPPORTED", True);

	if (x11_get_property(at, &args, &count)) {
		GP_DEBUG(1, "Window manager supports NetWM");

		for (i = 0; i < count; i++)
			x11_check_atoms(args[i]);

		XFree(args);
	}
}

static void x11_input_init(void);

static unsigned int x11_open(const char *display)
{
	if (x11_conn.ref_cnt != 0)
		return ++x11_conn.ref_cnt;

	GP_DEBUG(1, "Opening X11 display '%s'", display);

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
