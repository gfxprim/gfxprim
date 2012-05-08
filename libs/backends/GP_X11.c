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

#ifdef HAVE_LIBX11

#include <X11/Xlib.h>
#include <X11/Xutil.h>

#include "core/GP_Debug.h"
#include "input/GP_InputDriverX11.h"
#include "GP_X11.h"

struct x11_priv {
	Display *dpy;	
	int scr;
	Window win;
	Visual *vis;
	XImage *img;
};

static void x11_exit(GP_Backend *self)
{
	struct x11_priv *x11 = GP_BACKEND_PRIV(self); 
	
	XLockDisplay(x11->dpy);
	
	GP_ContextFree(self->context);

	x11->img->data = NULL;
	XDestroyImage(x11->img);
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
	          x11->img, x0, y0, x0, y0, x1-x0, y1-y0);
	XFlush(x11->dpy);

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

	XUnlockDisplay(x11->dpy);
}

static void x11_poll(GP_Backend *self)
{
	struct x11_priv *x11 = GP_BACKEND_PRIV(self); 
	XEvent ev;

	XLockDisplay(x11->dpy);
	
	while (XPending(x11->dpy)) {
		XNextEvent(x11->dpy, &ev);

		switch (ev.type) {
		case Expose:
			GP_DEBUG(4, "Expose %ix%i-%ix%i %i",
			         ev.xexpose.x, ev.xexpose.y,
			         ev.xexpose.width, ev.xexpose.height,
			         ev.xexpose.count);
			x11_update_rect(self, ev.xexpose.x, ev.xexpose.y,
			                ev.xexpose.x + ev.xexpose.width,
					ev.xexpose.y + ev.xexpose.height);
		break;
		case MapNotify:
			GP_DEBUG(1, "Shown");
		break;
		default:
			GP_InputDriverX11EventPut(&ev);
		break;
		}
	}
	
	XUnlockDisplay(x11->dpy);
}

static int x11_set_attributes(struct GP_Backend *self,
                              uint32_t w, uint32_t h,
                              const char *caption)
{
	struct x11_priv *x11 = GP_BACKEND_PRIV(self); 
	
	XLockDisplay(x11->dpy);
	
	if (caption != NULL)
		XmbSetWMProperties(x11->dpy, x11->win, caption, caption,
	                           NULL, 0, NULL, NULL, NULL);

	if (w != 0 || h != 0) {
		GP_Context *context;
		XImage *img;

		if (w == 0)
			w = self->context->w;
	
		if (h == 0)
			h = self->context->h;

		/* Create new X image */
		img = XCreateImage(x11->dpy, x11->vis, 24, ZPixmap, 0, NULL,
	                           w, h, 32, 0);

		/* Allocate new context */
		context = GP_ContextAlloc(w, h, GP_PIXEL_xRGB8888);
	
		if (context == NULL) {
			XDestroyImage(img);
			return 1;
		}

		/* Free old image and context */
		GP_ContextFree(self->context);
		x11->img->data = NULL;
		XDestroyImage(x11->img);

		/* Swap the pointers */
		self->context = context;
		img->data = (char*)self->context->pixels;
		x11->img = img;

		/* Resize X11 window */
		XResizeWindow(x11->dpy, x11->win, w, h);
		XFlush(x11->dpy);
	}
	
	XUnlockDisplay(x11->dpy);

	return 0;
}

GP_Backend *GP_BackendX11Init(const char *display, int x, int y,
                              unsigned int w, unsigned int h,
			      const char *caption)
{
	GP_Backend *backend;
	struct x11_priv *x11;

	GP_DEBUG(1, "Initalizing X11 display '%s'", display);

	backend = malloc(sizeof(GP_Backend) +
	                 sizeof(struct x11_priv));

	if (backend == NULL)
		return NULL;

	x11 = GP_BACKEND_PRIV(backend);

	backend->context = GP_ContextAlloc(w, h, GP_PIXEL_xRGB8888);

	if (backend->context == NULL)
		goto err0;
	
	//TODO: Error checking
	XInitThreads();

	x11->dpy = XOpenDisplay(display);

	if (x11->dpy == NULL)
		goto err1;

	x11->scr = DefaultScreen(x11->dpy);
	x11->vis = DefaultVisual(x11->dpy, x11->scr);
	
	GP_DEBUG(2, "Opening window '%s' %ix%i-%ux%u",
	         caption, x, y, w, h);
	
	x11->img = XCreateImage(x11->dpy, x11->vis, 24, ZPixmap, 0, NULL,
	                        w, h, 32, 0);

	x11->img->data = (char*)backend->context->pixels;

	x11->win = XCreateWindow(x11->dpy, DefaultRootWindow(x11->dpy),
	                         x, y, w, h, 0, CopyFromParent,
	                         InputOutput, CopyFromParent, 0, NULL);
	
	if (x11->win == None) {
		//TODO: Error message?
		GP_DEBUG(1, "Failed to create window");
		goto err2;
	}

	/* Select events */
	XSelectInput(x11->dpy, x11->win, StructureNotifyMask | ExposureMask |
	                                 KeyPressMask | KeyReleaseMask |
					 ButtonPressMask | ButtonReleaseMask |
					 PointerMotionMask);

	/* Set window caption */
	XmbSetWMProperties(x11->dpy, x11->win, caption, caption,
	                   NULL, 0, NULL, NULL, NULL);
	
	/* Show window */
	XMapWindow(x11->dpy, x11->win);
	XFlush(x11->dpy);

/*
	enum GP_PixelType pixel_type;
	pixel_type = GP_PixelRGBLookup(vscri.red.length,    vscri.red.offset,
	                               vscri.green.length,  vscri.green.offset,
	                               vscri.blue.length,   vscri.blue.offset,
	                               vscri.transp.length, vscri.transp.offset,
	                               vscri.bits_per_pixel);

	if (pixel_type == GP_PIXEL_UNKNOWN) {
		GP_DEBUG(1, "Unknown pixel type\n");
		goto err3;
	}


*/

	backend->name          = "X11";
	backend->Flip          = x11_flip;
	backend->UpdateRect    = x11_update_rect;
	backend->Exit          = x11_exit;
	backend->fd_list       = NULL;
	backend->Poll          = x11_poll;
	backend->SetAttributes = x11_set_attributes;

	return backend;
//err3:
//	XDestroyWindow(x11->dpy, x11->win);
err2:
	XCloseDisplay(x11->dpy);
err1:
	GP_ContextFree(backend->context);
err0:
	free(backend);
	return NULL;
}

#else

#include "GP_Backend.h"

GP_Backend *GP_BackendX11Init(const char *display, int x, int y,
                              unsigned int w, unsigned int h,
			      const char *caption)
{
	return NULL;
}

#endif /* HAVE_LIBX11 */
