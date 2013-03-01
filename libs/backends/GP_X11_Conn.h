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

	/* window reference counter */
	unsigned int ref_cnt;
};

static struct x11_conn x11_conn = {
	.dpy = NULL,
	.ref_cnt = 0,
};

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
	GP_InputDriverX11Init(x11_conn.dpy);

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
