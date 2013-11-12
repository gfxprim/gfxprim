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

#include "backends/GP_X11.h"

#include "GP_X11_Conn.h"
#include "GP_X11_Win.h"
#include "GP_X11_Input.h"

static int resize_ximage(GP_Backend *self, int w, int h);
static int resize_shm_ximage(GP_Backend *self, int w, int h);

static void putimage(struct x11_win *win, int x0, int y0, int x1, int y1)
{
#ifdef HAVE_X_SHM
	if (win->shm_flag)
		XShmPutImage(win->dpy, win->win, DefaultGC(win->dpy, win->scr),
		             win->img, x0, y0, x0, y0, x1-x0+1, y1-y0+1, False);
	else
#endif /* HAVE_X_SHM */
		XPutImage(win->dpy, win->win, DefaultGC(win->dpy, win->scr),
		          win->img, x0, y0, x0, y0, x1-x0+1, y1-y0+1);
}

static void x11_update_rect(GP_Backend *self, GP_Coord x0, GP_Coord y0,
                            GP_Coord x1, GP_Coord y1)
{
	struct x11_win *win = GP_BACKEND_PRIV(self);

	GP_DEBUG(4, "Updating rect %ix%i-%ix%i", x0, y0, x1, y1);

	if (win->resized_flag) {
		GP_DEBUG(4, "Ignoring update rect, waiting for resize ack");
		return;
	}

	XLockDisplay(win->dpy);

	putimage(win, x0, y0, x1, y1);

	XFlush(win->dpy);

	XUnlockDisplay(win->dpy);
}

static void x11_flip(GP_Backend *self)
{
	struct x11_win *win = GP_BACKEND_PRIV(self);
	unsigned int w = self->context->w;
	unsigned int h = self->context->h;

	GP_DEBUG(4, "Flipping context");

	if (win->resized_flag) {
		GP_DEBUG(4, "Ignoring flip, waiting for resize ack");
		return;
	}

	XLockDisplay(win->dpy);

	putimage(win, 0, 0, w - 1, h - 1);

	XFlush(win->dpy);

	XUnlockDisplay(win->dpy);
}

static struct x11_win *last_win = NULL;

static void x11_ev(XEvent *ev)
{
	struct x11_win *win;

	/* Lookup for window */
	if (last_win == NULL || last_win->win != ev->xany.window) {
		last_win = win_list_lookup(ev->xany.window);

		if (last_win == NULL) {
			GP_WARN("Event for unknown window, ignoring.");
			return;
		}
	}

	win = last_win;

	struct GP_Backend *self = GP_CONTAINER_OF(win, struct GP_Backend, priv);

	switch (ev->type) {
	case Expose:
		GP_DEBUG(4, "Expose %ix%i-%ix%i %i",
		         ev->xexpose.x, ev->xexpose.y,
		         ev->xexpose.width, ev->xexpose.height,
		         ev->xexpose.count);

		if (win->resized_flag)
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

		if (ev->xconfigure.width == (int)win->new_w &&
		    ev->xconfigure.height == (int)win->new_h)
			break;

		win->new_w = ev->xconfigure.width;
		win->new_h = ev->xconfigure.height;

		GP_DEBUG(4, "Configure Notify %ux%u", win->new_w, win->new_h);

		/*  Window has been resized, set flag. */
		win->resized_flag = 1;
	default:
		//TODO: More accurate window w and h?
		x11_input_event_put(&self->event_queue, ev,
		                    self->context->w, self->context->h);
	break;
	}
}

static void x11_poll(GP_Backend *self)
{
	struct x11_win *win = GP_BACKEND_PRIV(self);
	XEvent ev;

	XLockDisplay(win->dpy);

	while (XPending(win->dpy)) {
		XNextEvent(win->dpy, &ev);
		x11_ev(&ev);
	}

	XUnlockDisplay(win->dpy);
}

#include <poll.h>

static void x11_wait(GP_Backend *self)
{
	struct pollfd fd = {.fd = self->fd, .events = POLLIN, .revents = 0};
	poll(&fd, 1, -1);
	x11_poll(self);
}

static int resize_buffer(struct GP_Backend *self, uint32_t w, uint32_t h)
{
	struct x11_win *win = GP_BACKEND_PRIV(self);

	if (win->shm_flag) {
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
	struct x11_win *win = GP_BACKEND_PRIV(self);

	XLockDisplay(win->dpy);

	if (caption != NULL) {
		GP_DEBUG(3, "Setting window caption to '%s'", caption);
		XmbSetWMProperties(win->dpy, win->win, caption, caption,
	                           NULL, 0, NULL, NULL, NULL);
	}

	if (w != 0 && h != 0) {
		GP_DEBUG(3, "Setting window size to %ux%u", w, h);
		XResizeWindow(win->dpy, win->win, w, h);
	}

	XFlush(win->dpy);

	XUnlockDisplay(win->dpy);

	return 0;
}

static int x11_resize_ack(struct GP_Backend *self)
{
	struct x11_win *win = GP_BACKEND_PRIV(self);
	int ret;

	XLockDisplay(win->dpy);

	GP_DEBUG(3, "Setting buffer size to %ux%u", win->new_w, win->new_h);

	ret = resize_buffer(self, win->new_w, win->new_h);

	win->resized_flag = 0;

	if (!ret) {
		GP_EventQueueSetScreenSize(&self->event_queue,
		                           win->new_w, win->new_h);
	}

	GP_DEBUG(3, "Done");

	XUnlockDisplay(win->dpy);

	return ret;
}

static const char *visual_class_name(int class)
{
	switch (class) {
	case StaticGray:
		return "StaticGray";
	case GrayScale:
		return "GrayScale";
	case StaticColor:
		return "StaticColor";
	case PseudoColor:
		return "PseudoColor";
	case TrueColor:
		return "TrueColor";
	case DirectColor:
		return "DirectColor";
	}

	return "Unknown";
}

static enum GP_PixelType match_pixel_type(struct x11_win *win)
{
	GP_DEBUG(1, "Matching image pixel type, visual=%s depth=%u",
	         visual_class_name(win->vis->class), win->img->bits_per_pixel);

	if (win->vis->class == DirectColor || win->vis->class == TrueColor) {
		return GP_PixelRGBMatch(win->img->red_mask,
		                        win->img->green_mask,
		                        win->img->blue_mask,
		                        0x0, win->img->bits_per_pixel);
	}

	GP_FATAL("Unsupported visual %s", visual_class_name(win->vis->class));
	return GP_PIXEL_UNKNOWN;
}

#ifdef HAVE_X_SHM

static int create_shm_ximage(GP_Backend *self, GP_Size w, GP_Size h)
{
	struct x11_win *win = GP_BACKEND_PRIV(self);
	enum GP_PixelType pixel_type;

	if (XShmQueryExtension(win->dpy) == False) {
		GP_DEBUG(1, "MIT SHM Extension not supported, "
		            "falling back to XImage");
		return 1;
	}

	if (self->context == NULL)
		GP_DEBUG(1, "Using MIT SHM Extension");

	win->img = XShmCreateImage(win->dpy, win->vis, win->scr_depth,
	                           ZPixmap, NULL, &win->shminfo, w, h);

	if (win->img == NULL) {
		GP_WARN("Failed to create SHM XImage");
		return 1;
	}

	size_t size = win->img->bytes_per_line * win->img->height;

	pixel_type = match_pixel_type(win);

	if (pixel_type == GP_PIXEL_UNKNOWN) {
		GP_DEBUG(1, "Unknown pixel type");
		goto err0;
	}

	win->shminfo.shmid = shmget(IPC_PRIVATE, size, 0600);

	if (win->shminfo.shmid == -1) {
		GP_WARN("Calling shmget() failed: %s", strerror(errno));
		goto err0;
	}

	win->shminfo.shmaddr = win->img->data = shmat(win->shminfo.shmid, 0, 0);

	if (win->shminfo.shmaddr == (void *)-1) {
		GP_WARN("Calling shmat() failed: %s", strerror(errno));
		goto err1;
	}

	/* Mark SHM for deletion after detach */
	if (shmctl(win->shminfo.shmid, IPC_RMID, 0)) {
		GP_WARN("Calling shmctl(..., IPC_RMID), 0) failed: %s",
		         strerror(errno));
		goto err2;
	}

	win->shminfo.readOnly = False;

	if (XShmAttach(win->dpy, &win->shminfo) == False) {
		GP_WARN("XShmAttach failed");
		goto err2;
	}

	GP_ContextInit(&win->context, w, h, pixel_type, win->shminfo.shmaddr);
	win->context.bytes_per_row = win->img->bytes_per_line;

	self->context = &win->context;

	win->shm_flag = 1;

	//FIXME: Proper synchronization
	XSync(win->dpy, True);

	return 0;
err2:
	shmdt(win->shminfo.shmaddr);
err1:
	shmctl(win->shminfo.shmid, IPC_RMID, 0);
err0:
	XDestroyImage(win->img);
	return 1;
}

static void destroy_shm_ximage(GP_Backend *self)
{
	struct x11_win *win = GP_BACKEND_PRIV(self);

	XLockDisplay(win->dpy);

	XShmDetach(win->dpy, &win->shminfo);
	XFlush(win->dpy);
	shmdt(win->shminfo.shmaddr);
	XDestroyImage(win->img);
	XFlush(win->dpy);

	XUnlockDisplay(win->dpy);
}

static int resize_shm_ximage(GP_Backend *self, int w, int h)
{
	struct x11_win *win = GP_BACKEND_PRIV(self);
	int ret;

	GP_DEBUG(4, "Resizing XShmImage %ux%u -> %ux%u",
	         self->context->w, self->context->h, w, h);

	XLockDisplay(win->dpy);

	destroy_shm_ximage(self);
	ret = create_shm_ximage(self, w, h);

	XUnlockDisplay(win->dpy);

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
	return 1;
}

#endif /* HAVE_X_SHM */

static int create_ximage(GP_Backend *self, GP_Size w, GP_Size h)
{
	struct x11_win *win = GP_BACKEND_PRIV(self);
	enum GP_PixelType pixel_type;
	int depth;

	/* Get depth similiar to the default visual depth */
	switch (win->scr_depth) {
	case 32:
	case 24:
		depth = 32;
	case 16:
		depth = 16;
	case 8:
		depth = 8;
	default:
		/* TODO: better default */
		depth = 32;
	}

	GP_DEBUG(1, "Screen depth %i, using XImage depth %i",
	         win->scr_depth, depth);

	win->img = XCreateImage(win->dpy, win->vis, win->scr_depth, ZPixmap, 0,
	                       NULL, w, h, depth, 0);

	if (win->img == NULL) {
		GP_DEBUG(1, "Failed to create XImage");
		goto err0;
	}

	pixel_type = match_pixel_type(win);

	if (pixel_type == GP_PIXEL_UNKNOWN) {
		GP_DEBUG(1, "Unknown pixel type");
		goto err1;
	}

	self->context = GP_ContextAlloc(w, h, pixel_type);

	if (self->context == NULL) {
		GP_DEBUG(1, "Malloc failed :(");
		goto err1;
	}

	win->shm_flag = 0;
	win->img->data = (char*)self->context->pixels;

	return 0;
err1:
	XDestroyImage(win->img);
err0:
	return 1;
}

static void destroy_ximage(GP_Backend *self)
{
	struct x11_win *win = GP_BACKEND_PRIV(self);

	GP_ContextFree(self->context);
	win->img->data = NULL;
	XDestroyImage(win->img);
}

static int resize_ximage(GP_Backend *self, int w, int h)
{
	struct x11_win *win = GP_BACKEND_PRIV(self);
	XImage *img;

	/* Create new X image */
	img = XCreateImage(win->dpy, win->vis, win->scr_depth, ZPixmap, 0, NULL,
	                   w, h, win->img->bitmap_pad, 0);

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
	win->img->data = NULL;
	XDestroyImage(win->img);

	/* Swap the pointers */
	img->data = (char*)self->context->pixels;
	win->img = img;

	return 0;
}

static void window_close(GP_Backend *self)
{
	struct x11_win *win = GP_BACKEND_PRIV(self);

	XLockDisplay(win->dpy);

	if (win->shm_flag)
		destroy_shm_ximage(self);
	else
		destroy_ximage(self);

	x11_win_close(win);

	XUnlockDisplay(win->dpy);
}

static void x11_exit(GP_Backend *self)
{
	struct x11_win *win = GP_BACKEND_PRIV(self);

	GP_DEBUG(1, "Closing window %p", win);

	if (win == last_win)
		last_win = NULL;

	window_close(self);

	free(self);
}

GP_Backend *GP_BackendX11Init(const char *display, int x, int y,
                              unsigned int w, unsigned int h,
			      const char *caption,
			      enum GP_BackendX11Flags flags)
{
	GP_Backend *backend;
	struct x11_win *win;

	backend = malloc(sizeof(GP_Backend) +
	                 sizeof(struct x11_win));

	if (backend == NULL)
		return NULL;

	win = GP_BACKEND_PRIV(backend);

	//XSynchronize(win->dpy, True);

	/* Pack parameters and open window */
	struct x11_wreq wreq = {
		.win = win,
		.display = display,
		.x = x,
		.y = y,
		.w = w,
		.h = h,
		.caption = caption,
		.flags = flags,
	};

	x11_win_open(&wreq);

	if (win->win == None) {
		//TODO: Error message?
		GP_DEBUG(1, "Failed to create window");
		goto err1;
	}

	if (flags & GP_X11_FULLSCREEN)
		x11_win_fullscreen(win, 1);

	/* Init the event queue, once we know the window size */
	GP_EventQueueInit(&backend->event_queue, wreq.w, wreq.h, 0);

	backend->context = NULL;

	if ((flags & GP_X11_DISABLE_SHM) || create_shm_ximage(backend, w, h))
		if (create_ximage(backend, w, h))
			goto err1;

	XFlush(win->dpy);

	win->resized_flag = 0;

	backend->name          = "X11";
	backend->Flip          = x11_flip;
	backend->UpdateRect    = x11_update_rect;
	backend->Exit          = x11_exit;
	backend->Poll          = x11_poll;
	backend->Wait          = x11_wait;
	backend->SetAttributes = x11_set_attributes;
	backend->ResizeAck     = x11_resize_ack;
	backend->fd            = XConnectionNumber(win->dpy);
	backend->timers        = NULL;

	return backend;
err1:
	x11_close();
	free(backend);
	return NULL;
}

void GP_BackendX11RequestFullscreen(GP_Backend *self, int mode)
{
	struct x11_win *win = GP_BACKEND_PRIV(self);

	x11_win_fullscreen(win, mode);
}

#else

#include "GP_Backend.h"

GP_Backend *GP_BackendX11Init(const char *GP_UNUSED(display),
                              int GP_UNUSED(x), int GP_UNUSED(y),
                              unsigned int GP_UNUSED(w),
			      unsigned int GP_UNUSED(h),
			      const char *GP_UNUSED(caption))
{
	GP_FATAL("X11 support not compiled in");
	return NULL;
}

void GP_BackendX11RequestFullscreen(GP_Backend *GP_UNUSED(self),
                                    int GP_UNUSED(mode))
{
}

#endif /* HAVE_LIBX11 */

int GP_BackendIsX11(GP_Backend *self)
{
	return !strcmp(self->name, "X11");
}
