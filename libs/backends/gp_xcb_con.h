// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2009-2024 Cyril Hrubis <metan@ucw.cz>
 */

/*
 * XCB connection, singleton.
 */
struct x_con {
	xcb_connection_t *c;

	xcb_atom_t net_wm_state;
	xcb_atom_t net_wm_state_fullscreen;

	xcb_atom_t wm_protocols;
	xcb_atom_t wm_delete_window;

	xcb_cursor_t cursor_arrow;
	xcb_cursor_t cursor_text_edit;
	xcb_cursor_t cursor_crosshair;
	xcb_cursor_t cursor_hand;

	int shm_completion_ev;

	int fullscreen_supported:1;
	int state_supported:1;
	int delete_window_supported:1;
	int shm_support:1;
	int local:1;

	/* reference counter, incremented on window creation */
	unsigned int ref_cnt;
};

static struct x_con x_con = {
	.c = NULL,
	.ref_cnt = 0,
};

static void x_check_shm(xcb_connection_t *c)
{
	xcb_shm_query_version_reply_t *rep;
	xcb_shm_query_version_cookie_t ck;

	ck = xcb_shm_query_version(c);
	rep = xcb_shm_query_version_reply(c, ck, NULL);
	if (!rep) {
		GP_WARN("Failed to query SHM version");
		return;
	}

	GP_DEBUG(4, "SHM version %i.%i",
	         rep->major_version, rep->minor_version);

	if ((rep->major_version < 1) ||
	    (rep->major_version == 1 && rep->minor_version == 0)) {
		GP_DEBUG(1, "SHM version too old, needs at least v1.0");
		goto ret;
	}
/*
	if (rep->pixmap_format != XCB_IMAGE_FORMAT_Z_PIXMAP) {
		GP_DEBUG(1, "Wrong SHM pixmap format %u expected %u",
		         rep->pixmap_format, XCB_IMAGE_FORMAT_Z_PIXMAP);
		goto ret;
	}
*/
	x_con.shm_support = 1;
	x_con.shm_completion_ev = xcb_get_extension_data(c, &xcb_shm_id)->first_event + XCB_SHM_COMPLETION;
ret:
	free(rep);
}

static void x_check_x_fixes(xcb_connection_t *c)
{
	xcb_xfixes_query_version_reply_t *rep;
	xcb_xfixes_query_version_cookie_t ck;

	ck = xcb_xfixes_query_version(c, XCB_XFIXES_MAJOR_VERSION, XCB_XFIXES_MINOR_VERSION);
	rep = xcb_xfixes_query_version_reply(c, ck, NULL);
	if (!rep) {
		GP_WARN("Failed to query xfixes version");
		return;
	}

	GP_DEBUG(4, "xfixes version %i.%i",
	         rep->major_version, rep->minor_version);

	free(rep);
}

static int x_get_atom(xcb_connection_t *c, const char *name, xcb_atom_t *ret)
{
	xcb_intern_atom_cookie_t cookie;

	cookie = xcb_intern_atom(c, 0, strlen(name), name);

	xcb_intern_atom_reply_t *reply = xcb_intern_atom_reply(c, cookie, NULL);

	if (!reply)
		return 0;

	*ret = reply->atom;
	free(reply);

	return 1;
}

static void x_detect_wm_features(xcb_connection_t *c)
{
	xcb_atom_t at;

	if (!x_get_atom(c, "_NET_WM_STATE", &at)) {
		GP_DEBUG(1, "_NET_WM_STATE is not supported");
	} else {
		x_con.net_wm_state = at;
		x_con.state_supported = 1;
	}

	if (!x_get_atom(c, "_NET_WM_STATE_FULLSCREEN", &at)) {
		GP_DEBUG(1, "_NET_WM_STATE_FULLSCREEN is not supported");
	} else {
		x_con.net_wm_state_fullscreen = at;
		x_con.fullscreen_supported = 1;
	}

	if (x_get_atom(c, "WM_PROTOCOLS", &at)) {
		x_con.wm_protocols = at;
		if (x_get_atom(c, "WM_DELETE_WINDOW", &at)) {
			x_con.wm_delete_window = at;
			x_con.delete_window_supported = 1;
		}
	}

}

#define CURSOR_LEFT_PTR 68
#define CURSOR_XTERM 152
#define CURSOR_TCROSS 130
#define CURSOR_HAND2 60

static void x_load_cursors(xcb_connection_t *c)
{
	xcb_font_t font = xcb_generate_id(c);

	xcb_open_font(c, font, strlen("cursor"), "cursor");

	x_con.cursor_arrow = xcb_generate_id(c);
	x_con.cursor_text_edit = xcb_generate_id(c);
	x_con.cursor_crosshair = xcb_generate_id(c);
	x_con.cursor_hand = xcb_generate_id(c);

	xcb_create_glyph_cursor(c, x_con.cursor_arrow, font, font,
	                        CURSOR_LEFT_PTR, CURSOR_LEFT_PTR+1,
                                0, 0, 0, 65535, 65535, 65535);

	xcb_create_glyph_cursor(c, x_con.cursor_text_edit, font, font,
	                        CURSOR_XTERM, CURSOR_XTERM+1,
                                0, 0, 0, 65535, 65535, 65535);

	xcb_create_glyph_cursor(c, x_con.cursor_crosshair, font, font,
	                        CURSOR_TCROSS, CURSOR_TCROSS+1,
                                0, 0, 0, 65535, 65535, 65535);

	xcb_create_glyph_cursor(c, x_con.cursor_hand, font, font,
	                        CURSOR_HAND2, CURSOR_HAND2+1,
                                0, 0, 0, 65535, 65535, 65535);
}

static void xcb_input_init(xcb_connection_t *c);

static void check_local_connection(const char *display)
{
	if (!display)
		display = getenv("DISPLAY");

	if (!display) {
		GP_WARN("No DISPLAY set!");
		return;
	}

	if (display[0] == ':')
		x_con.local = 1;

	GP_DEBUG(1, "Connection is %s (%s)",
	         x_con.local ? "local" : "remote", display);
}

static unsigned int x_connect(const char *display)
{
	if (x_con.ref_cnt != 0)
		return ++x_con.ref_cnt;

	check_local_connection(display);

	GP_DEBUG(1, "Opening X display '%s'", display);

	x_con.c = xcb_connect(display, NULL);

	if (xcb_connection_has_error(x_con.c)) {
		GP_WARN("Failed to initialize X display");
		xcb_disconnect(x_con.c);
		return 0;
	}

	/* Initialized key translation table */
	xcb_input_init(x_con.c);

	if (x_con.local)
		x_check_shm(x_con.c);

	x_detect_wm_features(x_con.c);
	x_check_x_fixes(x_con.c);
	x_load_cursors(x_con.c);

	return ++x_con.ref_cnt;
}

static void x_close(void)
{
	/* Ignore close requests if connection is closed */
	if (x_con.ref_cnt == 0)
		return;

	if (--x_con.ref_cnt != 0)
		return;

	GP_DEBUG(1, "Closing X display");

	xcb_disconnect(x_con.c);
}
