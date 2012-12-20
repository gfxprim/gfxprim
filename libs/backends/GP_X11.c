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

#include <string.h>
#include <errno.h>

#include "../../config.h"

#include "core/GP_Debug.h"
#include "core/GP_Common.h"

#ifdef HAVE_LIBX11

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <X11/Xmd.h>

#ifdef HAVE_X_SHM
#include <sys/ipc.h>
#include <sys/shm.h>
#include <X11/extensions/XShm.h>
#endif /* HAVE_X_SHM */

#include "input/GP_InputDriverX11.h"
#include "GP_X11.h"

struct x11_priv {
	/* Connection details */
	Display *dpy;	
	int scr;
	Screen *scr_ptr;
	int scr_depth;
	Window win;
	Visual *vis;

	/* Image details */
	XImage *img;

#ifdef HAVE_X_SHM
	XShmSegmentInfo shminfo;
	GP_Context context;
#endif /* HAVE_X_SHM */

	int resized_flag:1;
	int shm_flag:1;

	/* used to store width and height from ConfigureNotify event */
	unsigned int new_w;
	unsigned int new_h;
};

static int resize_ximage(GP_Backend *self, int w, int h);
static int resize_shm_ximage(GP_Backend *self, int w, int h);

static void putimage(struct GP_Backend *self, int x0, int y0, int x1, int y1)
{
	struct x11_priv *x11 = GP_BACKEND_PRIV(self); 

#ifdef HAVE_X_SHM
	if (x11->shm_flag)
		XShmPutImage(x11->dpy, x11->win, DefaultGC(x11->dpy, x11->scr),
		             x11->img, x0, y0, x0, y0, x1-x0+1, y1-y0+1, False);
	else
#endif /* HAVE_X_SHM */
		XPutImage(x11->dpy, x11->win, DefaultGC(x11->dpy, x11->scr),
		          x11->img, x0, y0, x0, y0, x1-x0+1, y1-y0+1);
}

static void x11_update_rect(GP_Backend *self, GP_Coord x0, GP_Coord y0,
                            GP_Coord x1, GP_Coord y1)
{
	struct x11_priv *x11 = GP_BACKEND_PRIV(self); 
	
	GP_DEBUG(4, "Updating rect %ix%i-%ix%i", x0, y0, x1, y1);
	
	if (x11->resized_flag) {
		GP_DEBUG(4, "Ignoring update rect, waiting for resize ack");
		return;
	}
	
	XLockDisplay(x11->dpy);

	putimage(self, x0, y0, x1, y1);

	XUnlockDisplay(x11->dpy);
}

static void x11_flip(GP_Backend *self)
{
	struct x11_priv *x11 = GP_BACKEND_PRIV(self); 
	unsigned int w = self->context->w;
	unsigned int h = self->context->h;

	GP_DEBUG(4, "Flipping context");
	
	if (x11->resized_flag) {
		GP_DEBUG(4, "Ignoring flip, waiting for resize ack");
		return;
	}
	
	XLockDisplay(x11->dpy);

#ifdef HAVE_X_SHM
	if (x11->shm_flag)
		XShmPutImage(x11->dpy, x11->win, DefaultGC(x11->dpy, x11->scr),
		             x11->img, 0, 0, 0, 0, w, h, False);
	else
#endif /* HAVE_X_SHM */
		XPutImage(x11->dpy, x11->win, DefaultGC(x11->dpy, x11->scr),
		          x11->img, 0, 0, 0, 0, w, h);
	
	XFlush(x11->dpy);
	
	XUnlockDisplay(x11->dpy);
}

static void x11_ev(GP_Backend *self, XEvent *ev)
{
	struct x11_priv *x11 = GP_BACKEND_PRIV(self); 
	
	switch (ev->type) {
	case Expose:
		GP_DEBUG(4, "Expose %ix%i-%ix%i %i",
		         ev->xexpose.x, ev->xexpose.y,
		         ev->xexpose.width, ev->xexpose.height,
		         ev->xexpose.count);

		if (x11->resized_flag)
			break;

		/* Safety measure */
		if (ev->xexpose.x + ev->xexpose.width > (int)self->context->w) {
			GP_WARN("Expose x + w > context->w");
			break;
		}
		
		if (ev->xexpose.y + ev->xexpose.height > (int)self->context->h) {
			GP_WARN("Expose y + h > context->h");
			break;
		}

		/* Update the rectangle  */
		x11_update_rect(self, ev->xexpose.x, ev->xexpose.y,
		                ev->xexpose.x + ev->xexpose.width - 1,
				ev->xexpose.y + ev->xexpose.height - 1);
	break;
	case ConfigureNotify:
		if (ev->xconfigure.width == (int)self->context->w &&
		    ev->xconfigure.height == (int)self->context->h)
		    	break;
		
		if (ev->xconfigure.width == (int)x11->new_w &&
		    ev->xconfigure.height == (int)x11->new_h)
		    	break;
		
		x11->new_w = ev->xconfigure.width;
		x11->new_h = ev->xconfigure.height;

		GP_DEBUG(4, "Configure Notify %ux%u", x11->new_w, x11->new_h);

		/*  Window has been resized, set flag. */
		x11->resized_flag = 1;
	default:
		GP_InputDriverX11EventPut(ev);
	break;
	}
}

static void x11_poll(GP_Backend *self)
{
	struct x11_priv *x11 = GP_BACKEND_PRIV(self); 
	XEvent ev;

	XLockDisplay(x11->dpy);
	
	while (XPending(x11->dpy)) {
		XNextEvent(x11->dpy, &ev);
		x11_ev(self, &ev);
	}
	
	XUnlockDisplay(x11->dpy);
}

static void x11_wait(GP_Backend *self)
{
	struct x11_priv *x11 = GP_BACKEND_PRIV(self); 
	XEvent ev;

	XLockDisplay(x11->dpy);
	
	XNextEvent(x11->dpy, &ev);
	x11_ev(self, &ev);
	
	XUnlockDisplay(x11->dpy);
}

static int resize_buffer(struct GP_Backend *self, uint32_t w, uint32_t h)
{
	struct x11_priv *x11 = GP_BACKEND_PRIV(self); 
	
	if (x11->shm_flag) {
		if (resize_shm_ximage(self, w, h))
			return 1;
	} else {
		if (resize_ximage(self, w, h))
			return 1;
	}

	return 0;
}

static int x11_set_attributes(struct GP_Backend *self,
                              uint32_t w, uint32_t h,
                              const char *caption)
{
	struct x11_priv *x11 = GP_BACKEND_PRIV(self); 
	
	XLockDisplay(x11->dpy);
	
	if (caption != NULL) {
		GP_DEBUG(3, "Setting window caption to '%s'", caption);
		XmbSetWMProperties(x11->dpy, x11->win, caption, caption,
	                           NULL, 0, NULL, NULL, NULL);
	}

	if (w != 0 && h != 0) {
		GP_DEBUG(3, "Setting window size to %ux%u", w, h);
		XResizeWindow(x11->dpy, x11->win, w, h);
	}
	
	XUnlockDisplay(x11->dpy);

	return 0;
}

static int x11_resize_ack(struct GP_Backend *self)
{
	struct x11_priv *x11 = GP_BACKEND_PRIV(self); 
	int ret;

	XLockDisplay(x11->dpy);

	GP_DEBUG(3, "Setting buffer size to %ux%u", x11->new_w, x11->new_h);

	ret = resize_buffer(self, x11->new_w, x11->new_h);
	
	x11->resized_flag = 0;

	GP_DEBUG(3, "Done");

	XUnlockDisplay(x11->dpy);

	return ret;
}

static void match_pixel_type(struct x11_priv *x11,
                             enum GP_PixelType *pixel_type, int *depth)
{
	/*
	 * Eh, the XImage supports either 8, 16 or 32 bit pixels
	 *
	 * Do best effor on selecting appropriate pixel type
	 */
	for (*depth = 8; *depth <= 32; *depth<<=1) {
		*pixel_type = GP_PixelRGBMatch(x11->vis->red_mask,
		                               x11->vis->green_mask,
	                                       x11->vis->blue_mask,
		                               0x0, *depth);
	
		if (*pixel_type != GP_PIXEL_UNKNOWN)
			break;
	}
}

#ifdef HAVE_X_SHM

static int create_shm_ximage(GP_Backend *self, GP_Size w, GP_Size h)
{
	struct x11_priv *x11 = GP_BACKEND_PRIV(self);

	if (XShmQueryExtension(x11->dpy) == False) {
		GP_DEBUG(1, "MIT SHM Extension not supported, "
		            "falling back to XImage");
		return 1;
	}

	if (self->context == NULL)
		GP_DEBUG(1, "Using MIT SHM Extension");

	enum GP_PixelType pixel_type;
	int depth;

	if (self->context == NULL)
		match_pixel_type(x11, &pixel_type, &depth);
	else
		pixel_type = self->context->pixel_type;
	
	if (pixel_type == GP_PIXEL_UNKNOWN) {
		GP_DEBUG(1, "Unknown pixel type");
		return 1;
	}

	x11->img = XShmCreateImage(x11->dpy, x11->vis, x11->scr_depth,
	                           ZPixmap, NULL, &x11->shminfo, w, h);

	if (x11->img == NULL) {
		GP_WARN("Failed to create SHM XImage");
		return 1;
	}
	
	size_t size = x11->img->bytes_per_line * x11->img->height;

	x11->shminfo.shmid = shmget(IPC_PRIVATE, size, 0600);

	if (x11->shminfo.shmid == -1) {
		GP_WARN("Calling shmget() failed: %s", strerror(errno));
		goto err0;
	}
	
	x11->shminfo.shmaddr = x11->img->data = shmat(x11->shminfo.shmid, 0, 0);

	if (x11->shminfo.shmaddr == (void *)-1) {
		GP_WARN("Calling shmat() failed: %s", strerror(errno));
		goto err1;
	}

	/* Mark SHM for deletion after detach */
	if (shmctl(x11->shminfo.shmid, IPC_RMID, 0)) {
		GP_WARN("Calling shmctl(..., IPC_RMID), 0) failed: %s",
		         strerror(errno));
		goto err2;
	}
	
	x11->shminfo.readOnly = False;

	if (XShmAttach(x11->dpy, &x11->shminfo) == False) {
		GP_WARN("XShmAttach failed");
		goto err2;
	}

	GP_ContextInit(&x11->context, w, h, pixel_type, x11->shminfo.shmaddr);
	x11->context.bytes_per_row = x11->img->bytes_per_line;

	self->context = &x11->context;

	x11->shm_flag = 1;

	//FIXME: Proper synchronization
	XSync(x11->dpy, True);

	return 0;
err2:
	shmdt(x11->shminfo.shmaddr);
err1:
	shmctl(x11->shminfo.shmid, IPC_RMID, 0);
err0:
	XDestroyImage(x11->img);
	return 1;
}

static void destroy_shm_ximage(GP_Backend *self)
{
	struct x11_priv *x11 = GP_BACKEND_PRIV(self); 

	XLockDisplay(x11->dpy);
	
	XShmDetach(x11->dpy, &x11->shminfo);
	XFlush(x11->dpy);
	shmdt(x11->shminfo.shmaddr);
	XDestroyImage(x11->img);
	XFlush(x11->dpy);
	
	XUnlockDisplay(x11->dpy);
}

static int resize_shm_ximage(GP_Backend *self, int w, int h)
{
	struct x11_priv *x11 = GP_BACKEND_PRIV(self); 
	int ret;
	
	GP_DEBUG(4, "Resizing XShmImage %ux%u -> %ux%u",
	         self->context->w, self->context->h, w, h);
	
	XLockDisplay(x11->dpy);
	
	destroy_shm_ximage(self);
	ret = create_shm_ximage(self, w, h);
	
	XUnlockDisplay(x11->dpy);
	
	return ret;
}

#else

static int create_shm_ximage(GP_Backend GP_UNUSED(*self),
                             GP_Size GP_UNUSED(w), GP_Size GP_UNUSED(h))
{
	return 1;
}

static void destroy_shm_ximage(GP_Backend GP_UNUSED(*self))
{
	GP_WARN("Stub called");
}

static int resize_shm_ximage(GP_Backend GP_UNUSED(*self),
                             int GP_UNUSED(w), int GP_UNUSED(h))
{
	GP_WARN("Stub called");
}

#endif /* HAVE_X_SHM */

static int create_ximage(GP_Backend *self, GP_Size w, GP_Size h)
{
	struct x11_priv *x11 = GP_BACKEND_PRIV(self); 
	enum GP_PixelType pixel_type;
	int depth;

	match_pixel_type(x11, &pixel_type, &depth);
	
	if (pixel_type == GP_PIXEL_UNKNOWN) {
		GP_DEBUG(1, "Unknown pixel type");
		return 1;
	}

	self->context = GP_ContextAlloc(w, h, pixel_type);

	if (self->context == NULL)
		return 1;

	x11->img = XCreateImage(x11->dpy, x11->vis, x11->scr_depth, ZPixmap, 0,
	                       NULL, w, h, depth, 0);

	if (x11->img == NULL) {
		GP_DEBUG(1, "Failed to create XImage");
		GP_ContextFree(self->context);
		return 1;
	}

	x11->shm_flag = 0;

	x11->img->data = (char*)self->context->pixels;
	
	return 0;
}

static void destroy_ximage(GP_Backend *self)
{
	struct x11_priv *x11 = GP_BACKEND_PRIV(self); 

	GP_ContextFree(self->context);
	x11->img->data = NULL;
	XDestroyImage(x11->img);
}

static int resize_ximage(GP_Backend *self, int w, int h)
{
	struct x11_priv *x11 = GP_BACKEND_PRIV(self); 
	XImage *img;

	/* Create new X image */
	img = XCreateImage(x11->dpy, x11->vis, x11->scr_depth, ZPixmap, 0, NULL,
	                   w, h, x11->img->bitmap_pad, 0);

	if (img == NULL) {
		GP_DEBUG(2, "XCreateImage failed");
		return 1;
	}

	/* Resize context */
	if (GP_ContextResize(self->context, w, h)) {
		XDestroyImage(img);
		return 1;
	}

	/* Free old image */
	x11->img->data = NULL;
	XDestroyImage(x11->img);

	/* Swap the pointers */
	img->data = (char*)self->context->pixels;
	x11->img = img;

	return 0;
}

static void window_close(GP_Backend *self)
{
	struct x11_priv *x11 = GP_BACKEND_PRIV(self); 
	
	XLockDisplay(x11->dpy);

	if (x11->shm_flag)
		destroy_shm_ximage(self);
	else
		destroy_ximage(self);

	XDestroyWindow(x11->dpy, x11->win);
	
	XUnlockDisplay(x11->dpy);
}

static void x11_exit(GP_Backend *self)
{
	struct x11_priv *x11 = GP_BACKEND_PRIV(self); 
	
	XLockDisplay(x11->dpy);
	
	window_close(self);

	/* I wonder if this is right sequence... */
	//XUnlockDisplay(x11->dpy);
	XCloseDisplay(x11->dpy);

	free(self);
}

static void create_window(struct x11_priv *x11, int x, int y,
                          unsigned int *w, unsigned int *h,
                          const char *caption, enum GP_BackendX11Flags flags)
{
	XSetWindowAttributes attrs;
	unsigned long attr_mask = 0;
	
	/* Set event mask */
	attrs.event_mask = ExposureMask | StructureNotifyMask | KeyPressMask |
	                   KeyReleaseMask | PointerMotionMask;
	attr_mask |= CWEventMask;
	
	/* 
	 * If root window was selected, resize w and h and set x11->win to root
	 * window.
	 */
	if (flags & GP_X11_USE_ROOT_WIN) {
		x11->win = DefaultRootWindow(x11->dpy);
		*w = DisplayWidth(x11->dpy, x11->scr);
		*h = DisplayHeight(x11->dpy, x11->scr);
		
		GP_DEBUG(2, "Using root window, owerriding size to %ux%u",
		         *w, *h);

		XChangeWindowAttributes(x11->dpy, x11->win, attr_mask, &attrs);
		
		return;
	}
	
	/* 
	 * For some reason reading mouse button clicks on root win are not
	 * allowed...
	 */
	attrs.event_mask |= ButtonPressMask | ButtonReleaseMask;
	
	/*
	 * Create undecoreated root window on background
	 */
	if (flags & GP_X11_CREATE_ROOT_WIN) {
		Atom xa;
		
		*w = DisplayWidth(x11->dpy, x11->scr);
		*h = DisplayHeight(x11->dpy, x11->scr);
		
		GP_DEBUG(2, "Creating a window above root, owerriding size to %ux%u",
		         *w, *h);

		x11->win = XCreateWindow(x11->dpy, DefaultRootWindow(x11->dpy),
		                         0, 0, *w, *h, 0, CopyFromParent,
					 InputOutput, CopyFromParent, attr_mask, &attrs);
		
		/* Set empty WM_PROTOCOLS */
		GP_DEBUG(2, "Setting empty MW_PROTOCOLS");
		XSetWMProtocols(x11->dpy, x11->win, NULL, 0);

		/* Set window type to desktop */
		xa = XInternAtom(x11->dpy, "_NET_WM_WINDOW_TYPE", False);
		
		if (xa != None) {
			GP_DEBUG(2, "Setting Atom _NET_WM_WINDOW_TYPE to _NET_WM_WINDOW_TYPE_DESKTOP");
		
			Atom xa_prop = XInternAtom(x11->dpy, "_NET_WM_WINDOW_TYPE_DESKTOP", False);
		
			XChangeProperty(x11->dpy, x11->win, xa, XA_ATOM, 32,
			                PropModeReplace, (unsigned char *) &xa_prop, 1);
		}
		
		/* Turn off window decoration */
		xa = XInternAtom(x11->dpy, "_MOTIF_WM_HINTS", False);
		
		if (xa != None) {
			GP_DEBUG(2, "Setting Atom _MOTIF_WM_HINTS to 2, 0, 0, 0, 0");
			
			long prop[5] = {2, 0, 0, 0, 0};

			XChangeProperty(x11->dpy, x11->win, xa, xa, 32,
                                        PropModeReplace, (unsigned char *) prop, 5);
		}
		
		/* Set below other windows */
		xa = XInternAtom(x11->dpy, "_WIN_LAYER", False);

		if (xa != None) {
			GP_DEBUG(2, "Setting Atom _WIN_LAYER to 6");

			long prop = 6;

			XChangeProperty(x11->dpy, x11->win, xa, XA_CARDINAL, 32,
			                PropModeAppend, (unsigned char *) &prop, 1);
		}
		
		xa = XInternAtom(x11->dpy, "_NET_WM_STATE", False);

		if (xa != None) {
			GP_DEBUG(2, "Setting Atom _NET_WM_STATE to _NET_WM_STATE_BELOW");
			
			Atom xa_prop = XInternAtom(x11->dpy, "_NET_WM_STATE_BELOW", False);

			XChangeProperty(x11->dpy, x11->win, xa, XA_ATOM, 32,
			                PropModeAppend, (unsigned char *) &xa_prop, 1);
		}
		
		/* Set sticky */
		xa = XInternAtom(x11->dpy, "_NET_WM_DESKTOP", False);
		
		if (xa != None) {
			GP_DEBUG(2, "Setting Atom _NET_WM_DESKTOP to 0xffffffff");

			CARD32 xa_prop = 0xffffffff;
			
			XChangeProperty(x11->dpy, x11->win, xa, XA_CARDINAL, 32,
			                PropModeAppend, (unsigned char *) &xa_prop, 1);
		}

		xa = XInternAtom(x11->dpy, "_NET_WM_STATE", False);

		if (xa != None) {
			GP_DEBUG(2, "Appending to Atom _NET_WM_STATE atom _NET_WM_STATE_STICKY");

			Atom xa_prop = XInternAtom(x11->dpy, "_NET_WM_STATE_STICKY", False);

			XChangeProperty(x11->dpy, x11->win, xa, XA_ATOM, 32,
			                 PropModeAppend, (unsigned char *) &xa_prop, 1);
		}
		
		/* Skip taskbar */
		xa = XInternAtom(x11->dpy, "_NET_WM_STATE", False);
		
		if (xa != None) {
			GP_DEBUG(2, "Appending to Atom _NET_WM_STATE atom _NET_STATE_SKIP_TASKBAR");

			Atom xa_prop = XInternAtom(x11->dpy, "_NET_WM_STATE_SKIP_TASKBAR", False);

			XChangeProperty(x11->dpy, x11->win, xa, XA_ATOM, 32,
			                 PropModeAppend, (unsigned char *) &xa_prop, 1);
		}
		
		/* Skip pager */
		xa = XInternAtom(x11->dpy, "_NET_WM_STATE", False);
		
		if (xa != None) {
			GP_DEBUG(2, "Appending to Atom _NET_WM_STATE atom _NET_STATE_SKIP_PAGER");

			Atom xa_prop = XInternAtom(x11->dpy, "_NET_WM_STATE_SKIP_PAGER", False);

			 XChangeProperty(x11->dpy, x11->win, xa, XA_ATOM, 32,
			                 PropModeAppend, (unsigned char *) &xa_prop, 1);
		}
	
		/* Set 100% opacity */
		xa = XInternAtom(x11->dpy, "_NET_WM_WINDOW_OPACITY", False);

		if (xa != None) {
			GP_DEBUG(2, "Setting Atom _NET_WM_WINDOW_OPACITY to 0xffffffff");

			long prop = 0xffffffff;

			XChangeProperty(x11->dpy, x11->win, xa, XA_CARDINAL, 32,
			                PropModeAppend, (unsigned char *) &prop, 1);
		}

		/* Show window */
		XMapWindow(x11->dpy, x11->win);
		return;
	}

	GP_DEBUG(2, "Opening window '%s' %ix%i-%ux%u",
	         caption, x, y, *w, *h);

	x11->win = XCreateWindow(x11->dpy, DefaultRootWindow(x11->dpy),
	                         x, y, *w, *h, 0, CopyFromParent,
	                         InputOutput, CopyFromParent, attr_mask, &attrs);
	
	/* Set window caption */
	XmbSetWMProperties(x11->dpy, x11->win, caption, caption,
	                   NULL, 0, NULL, NULL, NULL);

	/* Make the window close button send event */
	Atom xa = XInternAtom(x11->dpy, "WM_DELETE_WINDOW", True);

	if (xa != None) {
		GP_DEBUG(2, "Setting WM_DELETE_WINDOW Atom to True");

		XSetWMProtocols(x11->dpy, x11->win, &xa, 1);
	} else {
		GP_DEBUG(2, "Failed to set WM_DELETE_WINDOW Atom to True");
	}
	
	/* Show window */
	XMapWindow(x11->dpy, x11->win);
}

#ifndef _NET_WM_STATE_ADD
# define _NET_WM_STATE_ADD 1
#endif /* _NET_WM_STATE_ADD */

#ifndef _NET_WM_STATE_REMOVE
# define _NET_WM_STATE_REMOVE 0
#endif /* _NET_WM_STATE_REMOVE */

/* Send NETWM message, most modern Window Managers should understand */
static void request_fullscreen(struct GP_Backend *self, int mode)
{
	struct x11_priv *x11 = GP_BACKEND_PRIV(self); 

	if (mode < 0 || mode > 2) {
		GP_WARN("Invalid fullscreen mode = %u", mode);
		return;
	}

	GP_DEBUG(2, "Requesting fullscreen mode = %u", mode);

	Atom wm_state, fullscreen;
	
	wm_state = XInternAtom(x11->dpy, "_NET_WM_STATE", True);
	fullscreen = XInternAtom(x11->dpy, "_NET_WM_STATE_FULLSCREEN", True);

	if (wm_state == None || fullscreen == None) {
		GP_WARN("Failed to create _NET_WM_* atoms");
		return;
	}

	XEvent ev;

	memset(&ev, 0, sizeof(ev));

	ev.type = ClientMessage;
	ev.xclient.window = x11->win;
	ev.xclient.message_type = wm_state;
	ev.xclient.format = 32;
	ev.xclient.data.l[0] = mode;
	ev.xclient.data.l[1] = fullscreen;
	ev.xclient.data.l[2] = 0;
	ev.xclient.data.l[3] = 1;

	if (!XSendEvent(x11->dpy, XDefaultRootWindow(x11->dpy), False, SubstructureNotifyMask, &ev)) {
		GP_WARN("Failed to send _NET_WM_STATE_FULLSCREEN event");
		return;
	}

	XFlush(x11->dpy);
}

GP_Backend *GP_BackendX11Init(const char *display, int x, int y,
                              unsigned int w, unsigned int h,
			      const char *caption,
			      enum GP_BackendX11Flags flags)
{
	GP_Backend *backend;
	struct x11_priv *x11;

	GP_DEBUG(1, "Initalizing X11 display '%s'", display);
	
	if (!XInitThreads()) {
		GP_DEBUG(2, "XInitThreads failed");
		return NULL;
	}

	backend = malloc(sizeof(GP_Backend) +
	                 sizeof(struct x11_priv));

	if (backend == NULL)
		return NULL;

	x11 = GP_BACKEND_PRIV(backend);

	x11->dpy = XOpenDisplay(display);

	if (x11->dpy == NULL)
		goto err0;

	//XSynchronize(x11->dpy, True);

	x11->scr = DefaultScreen(x11->dpy);
	x11->vis = DefaultVisual(x11->dpy, x11->scr);
	x11->scr_ptr = DefaultScreenOfDisplay(x11->dpy);
	x11->scr_depth = DefaultDepthOfScreen(x11->scr_ptr);

	GP_DEBUG(2, "Have Visual id %i, depth %u", (int)x11->vis->visualid, x11->scr_depth);

	create_window(x11, x, y, &w, &h, caption, flags);

	if (x11->win == None) {
		//TODO: Error message?
		GP_DEBUG(1, "Failed to create window");
		goto err1;
	}
	
	if (flags & GP_X11_FULLSCREEN)
		request_fullscreen(backend, 1);
	
	backend->context = NULL;

	if ((flags & GP_X11_DISABLE_SHM) || create_shm_ximage(backend, w, h))
		if (create_ximage(backend, w, h))
			goto err1;

	XFlush(x11->dpy);
	
	x11->resized_flag = 0;

	backend->name          = "X11";
	backend->Flip          = x11_flip;
	backend->UpdateRect    = x11_update_rect;
	backend->Exit          = x11_exit;
	backend->Poll          = x11_poll;
	backend->Wait          = x11_wait;
	backend->SetAttributes = x11_set_attributes;
	backend->ResizeAck     = x11_resize_ack;
	backend->fd            = XConnectionNumber(x11->dpy);

	return backend;
err1:
	XCloseDisplay(x11->dpy);
err0:
	free(backend);
	return NULL;
}

void GP_BackendX11RequestFullscreen(GP_Backend *self, int mode)
{
	return request_fullscreen(self, mode);
}

#else

#include "GP_Backend.h"

GP_Backend *GP_BackendX11Init(const char *GP_UNUSED(display),
                              int GP_UNUSED(x), int GP_UNUSED(y),
                              unsigned int GP_UNUSED(w),
			      unsigned int GP_UNUSED(h),
			      const char *GP_UNUSED(caption))
{
	GP_WARN("FATAL: X11 support not compiled in");
	return NULL;
}

void GP_BackendX11RequestFullscreen(GP_Backend *GP_UNUSED(self), int mode);

#endif /* HAVE_LIBX11 */

int GP_BackendIsX11(GP_Backend *self)
{
	return !strcmp(self->name, "X11");
}
