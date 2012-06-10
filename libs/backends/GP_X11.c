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

#include "../../config.h"

#include "core/GP_Debug.h"
#include "core/GP_Common.h"

#ifdef HAVE_LIBX11

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <X11/Xmd.h>

#include "input/GP_InputDriverX11.h"
#include "GP_X11.h"

struct x11_priv {
	Display *dpy;	
	int scr;
	Screen *scr_ptr;
	int scr_depth;
	Window win;
	Visual *vis;

	XImage *img;

	int resized_flag;
};

static void destroy_ximage(GP_Backend *self);

static int resize_ximage(GP_Backend *self, int w, int h);

static void x11_exit(GP_Backend *self)
{
	struct x11_priv *x11 = GP_BACKEND_PRIV(self); 
	
	XLockDisplay(x11->dpy);

	destroy_ximage(self);

	XDestroyWindow(x11->dpy, x11->win);
	/* I wonder if this is right sequence... */
	XUnlockDisplay(x11->dpy);
	XCloseDisplay(x11->dpy);

	free(self);
}

static void x11_update_rect(GP_Backend *self, GP_Coord x0, GP_Coord y0,
                            GP_Coord x1, GP_Coord y1)
{
	struct x11_priv *x11 = GP_BACKEND_PRIV(self); 
	
	GP_DEBUG(4, "Updating rect %ix%i-%ix%i", x0, y0, x1, y1);

	XLockDisplay(x11->dpy);

	XPutImage(x11->dpy, x11->win, DefaultGC(x11->dpy, x11->scr),
	          x11->img, x0, y0, x0, y0, x1-x0+1, y1-y0+1);
	XFlush(x11->dpy);

	x11->resized_flag = 0;

	XUnlockDisplay(x11->dpy);
}

static void x11_flip(GP_Backend *self)
{
	struct x11_priv *x11 = GP_BACKEND_PRIV(self); 
	unsigned int w = self->context->w;
	unsigned int h = self->context->h;

	GP_DEBUG(4, "Flipping context");

	XLockDisplay(x11->dpy);

	XPutImage(x11->dpy, x11->win, DefaultGC(x11->dpy, x11->scr),
	          x11->img, 0, 0, 0, 0, w, h);
	XFlush(x11->dpy);
	
	x11->resized_flag = 0;

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

		x11_update_rect(self, ev->xexpose.x, ev->xexpose.y,
		                ev->xexpose.x + ev->xexpose.width - 1,
				ev->xexpose.y + ev->xexpose.height - 1);
	break;
	case ConfigureNotify:
		if (ev->xconfigure.width == (int)self->context->w &&
		    ev->xconfigure.height == (int)self->context->h)
		    	break;
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
		
		if (resize_ximage(self, w, h))
			return 1;

		/* Resize X11 window */
	//	XResizeWindow(x11->dpy, x11->win, w, h);
		XFlush(x11->dpy);

		x11->resized_flag = 1;
	}
	
	XUnlockDisplay(x11->dpy);

	return 0;
}

static int create_ximage(GP_Backend *self, GP_Size w, GP_Size h)
{
	struct x11_priv *x11 = GP_BACKEND_PRIV(self); 
	int depth;
	enum GP_PixelType pixel_type;

	/*
	 * Eh, the XImage supports either 8, 16 or 32 bit pixels
	 *
	 * Do best effor on selecting appropriate pixel type
	 */
	for (depth = 8; depth <= 32; depth<<=1) {
		pixel_type = GP_PixelRGBMatch(x11->vis->red_mask,
		                              x11->vis->green_mask,
	                                      x11->vis->blue_mask,
					      0x0, depth);
	
		if (pixel_type != GP_PIXEL_UNKNOWN)
			break;
	}

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

void create_window(struct x11_priv *x11, int x, int y,
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
	
	/* Show window */
	XMapWindow(x11->dpy, x11->win);
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
	
	if (create_ximage(backend, w, h))
		goto err1;


	XFlush(x11->dpy);
	
	x11->resized_flag  = 0;

	backend->name          = "X11";
	backend->Flip          = x11_flip;
	backend->UpdateRect    = x11_update_rect;
	backend->Exit          = x11_exit;
	backend->Poll          = x11_poll;
	backend->Wait          = x11_wait;
	backend->SetAttributes = x11_set_attributes;
	backend->fd            = XConnectionNumber(x11->dpy);

	return backend;
err1:
	XCloseDisplay(x11->dpy);
err0:
	free(backend);
	return NULL;
}

#else

#include "GP_Backend.h"

GP_Backend *GP_BackendX11Init(const char *GP_UNUSED(display),
                              int GP_UNUSED(x), int GP_UNUSED(y),
                              unsigned int GP_UNUSED(w),
			      unsigned int GP_UNUSED(h),
			      const char *GP_UNUSED(caption))
{
	GP_DEBUG(0, "FATAL: X11 support not compiled in");
	return NULL;
}

#endif /* HAVE_LIBX11 */
