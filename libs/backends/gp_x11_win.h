// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2009-2013 Cyril Hrubis <metan@ucw.cz>
 */

/*
 * X11 window.
 */
struct x11_win {
	/* X11 Display */
	Display *dpy;
	/* X11 Screen */
	int scr;
	/* X11 Screen depth */
	int scr_depth;
	/* X11 Visual */
	Visual *vis;
	/* X11 Window */
	Window win;

	/* Unicode input */
	XIM xim;
	XIC xic;

	/* Image info SHM or XImage */
	XImage *img;

#ifdef HAVE_X_SHM
	XShmSegmentInfo shminfo;
#endif /* HAVE_X_SHM */

	gp_pixmap pixmap;
	gp_ev_queue ev_queue;

	/* Window list pointers */
	struct x11_win *prev;
	struct x11_win *next;

	/* Flags */
	int resized_flag:1;
	int shm_flag:1;
	int fullscreen_flag:1;
	int visible:1;
	int focused:1;

	/* poll fd structure */
	gp_fd fd;

	/* used to store width and height from ConfigureNotify event */
	unsigned int new_w;
	unsigned int new_h;
};

static struct x11_win *win_list = NULL;

static void win_list_add(struct x11_win *win)
{
	win->next = win_list;
	win->prev = NULL;
	win_list = win;
}

static void win_list_rem(struct x11_win *win)
{
	if (win->prev)
		win->prev->next = win->next;

	if (win->next)
		win->next->prev = win->prev;

	if (win == win_list)
		win_list = win->next;
}

static struct x11_win *win_list_lookup(Window win)
{
	struct x11_win *i;

	for (i = win_list; i != NULL; i = i->next) {
		if (i->win == win)
			return i;
	}

	return NULL;
}

static int win_list_empty(void)
{
	return win_list == NULL;
}

#ifndef _NET_WM_STATE_REMOVE
# define _NET_WM_STATE_REMOVE 0
#endif

#ifndef _NET_WM_STATE_ADD
# define _NET_WM_STATE_ADD 1
#endif

#ifndef _NET_WM_STATE_TOGGLE
# define _NET_WM_STATE_TOGGLE 2
#endif

/* Send NETWM message, most modern Window Managers should understand */
static enum gp_backend_ret x11_win_fullscreen(struct x11_win *win, enum gp_backend_fullscreen_req req)
{
	int fs;
	XEvent ev;
	int mode;

	switch (req) {
	case GP_BACKEND_FULLSCREEN_OFF:
		fs = 0;
		mode = _NET_WM_STATE_REMOVE;
	break;
	case GP_BACKEND_FULLSCREEN_ON:
		fs = 1;
		mode = _NET_WM_STATE_ADD;
	break;
	case GP_BACKEND_FULLSCREEN_TOGGLE:
		fs = !win->fullscreen_flag;
		mode = _NET_WM_STATE_TOGGLE;
	break;
	case GP_BACKEND_FULLSCREEN_QUERY:
		return !!win->fullscreen_flag;
	break;
	default:
		GP_WARN("Invalid fullscreen req = %i", (int)req);
		return GP_BACKEND_NOTSUPP;
	}

	if (fs == win->fullscreen_flag)
		return GP_BACKEND_OK;

	if (!x11_conn.S__NET_WM_STATE || !x11_conn.S__NET_WM_STATE_FULLSCREEN) {
		GP_WARN("NetWM Fullscreen not supported");
		return GP_BACKEND_NOTSUPP;
	}

	GP_DEBUG(2, "Requesting fullscreen mode = %i, fs = %i", (int)req, fs);

	memset(&ev, 0, sizeof(ev));

	ev.type = ClientMessage;
	ev.xclient.window = win->win;
	ev.xclient.message_type = x11_conn.A__NET_WM_STATE;
	ev.xclient.format = 32;
	ev.xclient.data.l[0] = mode;
	ev.xclient.data.l[1] = x11_conn.A__NET_WM_STATE_FULLSCREEN;
	ev.xclient.data.l[2] = 0;
	ev.xclient.data.l[3] = 1;

	if (!XSendEvent(win->dpy, XDefaultRootWindow(win->dpy),
	                False, SubstructureNotifyMask|SubstructureRedirectMask, &ev)) {
		GP_WARN("Failed to send _NET_WM_STATE_FULLSCREEN event");
		return GP_BACKEND_CONNERR;
	}

	win->fullscreen_flag = fs;
	XFlush(win->dpy);

	return GP_BACKEND_OK;
}

/* Window request structure */
struct x11_wreq {
	struct x11_win *win;

	/* X11 display */
	const char *display;

	/* geometry */
	int x;
	int y;
	unsigned int w;
	unsigned int h;

	const char *caption;

	unsigned int flags;
};

static void x11_get_screen_size(struct x11_wreq *wreq)
{
	wreq->w = DisplayWidth(wreq->win->dpy, wreq->win->scr);
	wreq->h = DisplayHeight(wreq->win->dpy, wreq->win->scr);
}

static void x11_input_init_im(struct x11_win *win);
static void x11_input_exit_im(struct x11_win *win);

static int x11_win_open(struct x11_wreq *wreq)
{
	XSetWindowAttributes attrs;
	unsigned long attr_mask = 0;
	struct x11_win *win;
	Screen *scr_ptr;

	/* Initialize connection/increase ref count */
	if (!x11_open(wreq->display))
		return 1;

	win = wreq->win;

	win->fullscreen_flag = 0;

	/* Copy display */
	win->dpy = x11_conn.dpy;

	/* Get visual and screen depth */
	win->scr = DefaultScreen(win->dpy);
	win->vis = DefaultVisual(win->dpy, win->scr);
	scr_ptr = DefaultScreenOfDisplay(win->dpy);
	win->scr_depth = DefaultDepthOfScreen(scr_ptr);

	GP_DEBUG(2, "Have Visual id %i, depth %u", (int)win->vis->visualid, win->scr_depth);

	/* Set event mask */
	attrs.event_mask = ExposureMask | StructureNotifyMask | KeyPressMask |
	                   KeyReleaseMask | PointerMotionMask | FocusChangeMask | VisibilityChangeMask;
	attr_mask |= CWEventMask;

	/*
	 * If root window was selected, resize w and h and set win->win to root
	 * window.
	 */
	if (wreq->flags & GP_X11_USE_ROOT_WIN) {

		win->win = DefaultRootWindow(win->dpy);

		x11_get_screen_size(wreq);

		GP_DEBUG(2, "Using root window, owerriding size to %ux%u",
		         wreq->w, wreq->h);

		win_list_add(win);

		XChangeWindowAttributes(win->dpy, win->win, attr_mask, &attrs);

		return 0;
	}

	/*
	 * For some reason reading mouse button clicks on root win are not
	 * allowed...
	 */
	attrs.event_mask |= ButtonPressMask | ButtonReleaseMask;

	/*
	 * Create undecoreated root window on background
	 */
	if (wreq->flags & GP_X11_CREATE_ROOT_WIN) {
		Atom xa;

		x11_get_screen_size(wreq);

		GP_DEBUG(2, "Creating a window above root, owerriding size to %ux%u",
		         wreq->w, wreq->h);

		win->win = XCreateWindow(win->dpy, DefaultRootWindow(win->dpy),
		                         0, 0, wreq->w, wreq->h, 0, CopyFromParent,
					 InputOutput, CopyFromParent, attr_mask, &attrs);

		/* Set empty WM_PROTOCOLS */
		GP_DEBUG(2, "Setting empty MW_PROTOCOLS");
		XSetWMProtocols(win->dpy, win->win, NULL, 0);

		/* Set window type to desktop */
		xa = XInternAtom(win->dpy, "_NET_WM_WINDOW_TYPE", False);

		if (xa != None) {
			GP_DEBUG(2, "Setting Atom _NET_WM_WINDOW_TYPE to _NET_WM_WINDOW_TYPE_DESKTOP");

			Atom xa_prop = XInternAtom(win->dpy, "_NET_WM_WINDOW_TYPE_DESKTOP", False);

			XChangeProperty(win->dpy, win->win, xa, XA_ATOM, 32,
			                PropModeReplace, (unsigned char *) &xa_prop, 1);
		}

		/* Turn off window decoration */
		xa = XInternAtom(win->dpy, "_MOTIF_WM_HINTS", False);

		if (xa != None) {
			GP_DEBUG(2, "Setting Atom _MOTIF_WM_HINTS to 2, 0, 0, 0, 0");

			long prop[5] = {2, 0, 0, 0, 0};

			XChangeProperty(win->dpy, win->win, xa, xa, 32,
                                        PropModeReplace, (unsigned char *) prop, 5);
		}

		/* Set below other windows */
		xa = XInternAtom(win->dpy, "_WIN_LAYER", False);

		if (xa != None) {
			GP_DEBUG(2, "Setting Atom _WIN_LAYER to 6");

			long prop = 6;

			XChangeProperty(win->dpy, win->win, xa, XA_CARDINAL, 32,
			                PropModeAppend, (unsigned char *) &prop, 1);
		}

		xa = XInternAtom(win->dpy, "_NET_WM_STATE", False);

		if (xa != None) {
			GP_DEBUG(2, "Setting Atom _NET_WM_STATE to _NET_WM_STATE_BELOW");

			Atom xa_prop = XInternAtom(win->dpy, "_NET_WM_STATE_BELOW", False);

			XChangeProperty(win->dpy, win->win, xa, XA_ATOM, 32,
			                PropModeAppend, (unsigned char *) &xa_prop, 1);
		}

		/* Set sticky */
		xa = XInternAtom(win->dpy, "_NET_WM_DESKTOP", False);

		if (xa != None) {
			GP_DEBUG(2, "Setting Atom _NET_WM_DESKTOP to 0xffffffff");

			CARD32 xa_prop = 0xffffffff;

			XChangeProperty(win->dpy, win->win, xa, XA_CARDINAL, 32,
			                PropModeAppend, (unsigned char *) &xa_prop, 1);
		}

		xa = XInternAtom(win->dpy, "_NET_WM_STATE", False);

		if (xa != None) {
			GP_DEBUG(2, "Appending to Atom _NET_WM_STATE atom _NET_WM_STATE_STICKY");

			Atom xa_prop = XInternAtom(win->dpy, "_NET_WM_STATE_STICKY", False);

			XChangeProperty(win->dpy, win->win, xa, XA_ATOM, 32,
			                 PropModeAppend, (unsigned char *) &xa_prop, 1);
		}

		/* Skip taskbar */
		xa = XInternAtom(win->dpy, "_NET_WM_STATE", False);

		if (xa != None) {
			GP_DEBUG(2, "Appending to Atom _NET_WM_STATE atom _NET_STATE_SKIP_TASKBAR");

			Atom xa_prop = XInternAtom(win->dpy, "_NET_WM_STATE_SKIP_TASKBAR", False);

			XChangeProperty(win->dpy, win->win, xa, XA_ATOM, 32,
			                 PropModeAppend, (unsigned char *) &xa_prop, 1);
		}

		/* Skip pager */
		xa = XInternAtom(win->dpy, "_NET_WM_STATE", False);

		if (xa != None) {
			GP_DEBUG(2, "Appending to Atom _NET_WM_STATE atom _NET_STATE_SKIP_PAGER");

			Atom xa_prop = XInternAtom(win->dpy, "_NET_WM_STATE_SKIP_PAGER", False);

			 XChangeProperty(win->dpy, win->win, xa, XA_ATOM, 32,
			                 PropModeAppend, (unsigned char *) &xa_prop, 1);
		}

		/* Set 100% opacity */
		xa = XInternAtom(win->dpy, "_NET_WM_WINDOW_OPACITY", False);

		if (xa != None) {
			GP_DEBUG(2, "Setting Atom _NET_WM_WINDOW_OPACITY to 0xffffffff");

			long prop = 0xffffffff;

			XChangeProperty(win->dpy, win->win, xa, XA_CARDINAL, 32,
			                PropModeAppend, (unsigned char *) &prop, 1);
		}

		win_list_add(win);

		/* Show window */
		XMapWindow(win->dpy, win->win);
		return 0;
	}

	GP_DEBUG(2, "Opening window '%s' %ix%i-%ux%u",
	         wreq->caption, wreq->x, wreq->y, wreq->w, wreq->h);

	win->win = XCreateWindow(win->dpy, DefaultRootWindow(win->dpy),
	                         wreq->x, wreq->y, wreq->w, wreq->h, 0,
	                         CopyFromParent, InputOutput, CopyFromParent,
	                         attr_mask, &attrs);

	/* Set window caption */
	XmbSetWMProperties(win->dpy, win->win, wreq->caption, wreq->caption,
	                   NULL, 0, NULL, NULL, NULL);

	/* Make the window close button send event */
	XSetWMProtocols(win->dpy, win->win, &x11_conn.A_WM_DELETE_WINDOW, 1);

	x11_input_init_im(win);

	win_list_add(win);

	return 0;
}

#ifdef HAVE_LIBXRANDR

#include <X11/extensions/Xrandr.h>

static inline unsigned int x11_win_get_dpi(struct x11_win *win)
{
	XRRScreenResources *screen_resource;
	XRRCrtcInfo *crtc_info;
	XRROutputInfo *output_info;

	screen_resource = XRRGetScreenResources(win->dpy, DefaultRootWindow(win->dpy));
	crtc_info = XRRGetCrtcInfo(win->dpy, screen_resource, screen_resource->crtcs[0]);
	output_info = XRRGetOutputInfo(win->dpy, screen_resource, screen_resource->outputs[0]);

	unsigned int dpi = gp_dpi_from_size(crtc_info->width, output_info->mm_width,
	                                    crtc_info->height, output_info->mm_height);

	XRRFreeOutputInfo(output_info);
	XRRFreeCrtcInfo(crtc_info);
	XRRFreeScreenResources(screen_resource);

	return dpi;
}
#else
static inline unsigned int x11_win_get_dpi(struct x11_win *win)
{
	(void) win;
	GP_WARN("Xrandr not compiled in");
	return 0;
}
#endif /* HAVE_LIBXRANDR */

static void x11_win_close(struct x11_win *win)
{
	GP_DEBUG(1, "Closing window");

	x11_input_exit_im(win);

	XLockDisplay(win->dpy);

	win_list_rem(win);

/*
	if (x11->shm_flag)
		destroy_shm_ximage(self);
	else
		destroy_ximage(self);
*/
	XUnmapWindow(win->dpy, win->win);

	XDestroyWindow(win->dpy, win->win);

	if (!win_list_empty())
		XFlush(win->dpy);

	XUnlockDisplay(win->dpy);

	/* Close connection/Decrease ref count */
	x11_close();
}
