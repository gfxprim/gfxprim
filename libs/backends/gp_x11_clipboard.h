// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2009-2021 Cyril Hrubis <metan@ucw.cz>
 */

#include <backends/gp_clipboard.h>

static void clear_clipboard(gp_backend *self)
{
	free(self->clipboard_data);
	self->clipboard_data = NULL;
}

static void copy_to_clipboard(gp_backend *self, const char *str, size_t len)
{
	free(self->clipboard_data);

	if (len)
		self->clipboard_data = strndup(str, len);
	else
		self->clipboard_data = strdup(str);
}

static int x11_clipboard(gp_backend *self, gp_clipboard *op)
{
	struct x11_win *win = GP_BACKEND_PRIV(self);

	switch (op->op) {
	case GP_CLIPBOARD_SET:
		copy_to_clipboard(self, op->str, op->len);
		XSetSelectionOwner(win->dpy, x11_conn.A_CLIPBOARD, win->win, 0);
		GP_DEBUG(3, "Setting XSelectionOwner for A_CLIPBOARD");
		goto flush;
	break;
	case GP_CLIPBOARD_CLEAR:
		clear_clipboard(self);
		XSetSelectionOwner(win->dpy, x11_conn.A_CLIPBOARD, None, 0);
		GP_DEBUG(3, "Clearing XSelectionOwner for A_CLIPBOARD");
                goto flush;
        break;
	case GP_CLIPBOARD_REQUEST:
		XConvertSelection(win->dpy, x11_conn.A_CLIPBOARD,
		                  x11_conn.A_UTF8_STRING, x11_conn.A_XSEL_DATA,
		                  win->win, CurrentTime);
		GP_DEBUG(3, "Requesting A_CLIPBOARD with XConvertSelection");
		goto flush;
	break;
	case GP_CLIPBOARD_GET:
		if (!self->clipboard_data) {
			GP_DEBUG(3, "No clipboard data available");
			op->ret = NULL;
			return 1;
		}
		GP_DEBUG(3, "Returning clipboard data");
		op->ret = strdup(self->clipboard_data);
		goto flush;
	break;
	}

	return 1;
flush:
	XFlush(win->dpy);
	return 0;
}

static void x11_selection_clear(gp_backend *self, XEvent *ev)
{
	if (ev->xselectionclear.selection != x11_conn.A_CLIPBOARD)
		return;

	GP_DEBUG(3, "XSelectionClear for A_CLIPBOARD");

	clear_clipboard(self);
}

static void x11_selection_request(gp_backend *self, XEvent *ev)
{
	struct x11_win *win = GP_BACKEND_PRIV(self);

	XSelectionEvent reply = {
		.type = SelectionNotify,
		.display = win->dpy,
		.requestor = ev->xselectionrequest.requestor,
		.selection = x11_conn.A_CLIPBOARD,
		.time = ev->xselectionrequest.time,
		.target = ev->xselectionrequest.target,
		.property = ev->xselectionrequest.property
	};

	if (ev->xselectionrequest.selection != x11_conn.A_CLIPBOARD)
		goto empty_reply;

	GP_DEBUG(3, "XSelectionRequest for A_CLIPBOARD");

	if (!self->clipboard_data) {
		GP_DEBUG(4, "Clipboad data no longer available");
		goto empty_reply;
	}

	if (ev->xselectionrequest.target == x11_conn.A_STRING ||
	    ev->xselectionrequest.target == x11_conn.A_TEXT) {
		GP_DEBUG(4, "Selection target A_STRING or A_TEXT");
		XChangeProperty(reply.display, reply.requestor, reply.property,
		                x11_conn.A_STRING, 8, PropModeReplace,
				self->clipboard_data, strlen(self->clipboard_data));
		goto reply;
	}

	if (ev->xselectionrequest.target == x11_conn.A_UTF8_STRING) {
		GP_DEBUG(4, "Selection target A_UTF8_STRING");
		XChangeProperty(reply.display, reply.requestor, reply.property,
		                x11_conn.A_UTF8_STRING, 8, PropModeReplace,
				self->clipboard_data, strlen(self->clipboard_data));
		goto reply;
	}

	if (ev->xselectionrequest.target == x11_conn.A_TARGETS) {
		GP_DEBUG(4, "Selection targets request -> reply A_UTF8_STRING");
		XChangeProperty(reply.display, reply.requestor, reply.property,
				x11_conn.A_ATOM, 32, PropModeReplace,
				(unsigned char*)&x11_conn.A_UTF8_STRING, 1);
		goto reply;
	}

	GP_DEBUG(4, "Unknown selection target ATOM %li", ev->xselection.target);

empty_reply:
	reply.property = None;
reply:
	XSendEvent(win->dpy, reply.requestor, 0, 0, (XEvent *)&reply);
}

static void x11_selection_notify(gp_backend *self, XEvent *ev)
{
	Atom target;
	int format;
	unsigned long N, size;
	char *data;

	if (ev->xselection.selection != x11_conn.A_CLIPBOARD)
		return;

	GP_DEBUG(3, "XSelectionNotify for A_CLIPBOARD");

	if (ev->xselection.property == None) {
		GP_DEBUG(4, "Got an empty reply");
		return;
	}

	XGetWindowProperty(ev->xselection.display, ev->xselection.requestor,
	                   ev->xselection.property, 0L, (~0L), 0, AnyPropertyType, &target,
	                   &format, &size, &N, (unsigned char**)&data);

	if (target == x11_conn.A_UTF8_STRING || target == x11_conn.A_STRING) {
		GP_DEBUG(4, "Got A_UTF8_STRING or A_STRING");
		copy_to_clipboard(self, data, size);
		XFree(data);
		gp_backend_clipboard_ready(self);
	} else {
		GP_DEBUG(4, "Unknown clipboard target %li", target);
	}

	XDeleteProperty(ev->xselection.display, ev->xselection.requestor, ev->xselection.property);
}
