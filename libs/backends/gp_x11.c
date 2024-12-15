// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2009-2024 Cyril Hrubis <metan@ucw.cz>
 */

#include <string.h>
#include <errno.h>

#include "../../config.h"

#include <core/gp_debug.h>
#include "core/gp_common.h"
#include "core/gp_pixmap.h"

#ifdef HAVE_LIBX11

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <X11/Xmd.h>
#include <X11/Xlocale.h>
#include <X11/cursorfont.h>
#include <X11/extensions/Xfixes.h>

#ifdef HAVE_X_SHM
#include <sys/ipc.h>
#include <sys/shm.h>
#include <X11/extensions/XShm.h>
#endif /* HAVE_X_SHM */

#include <backends/gp_x11.h>
#include <backends/gp_dpi.h>

#include "gp_x11_conn.h"
#include "gp_x11_win.h"
#include "gp_x11_input.h"
#include "gp_x11_clipboard.h"

static int resize_ximage(gp_backend *self, int w, int h);
static int resize_shm_ximage(gp_backend *self, int w, int h);
static void process_events(struct x11_win *win, gp_backend *backend);

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

static void x11_update_rect(gp_backend *self, gp_coord x0, gp_coord y0,
                            gp_coord x1, gp_coord y1)
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

	process_events(win, self);

	XUnlockDisplay(win->dpy);
}

static void x11_flip(gp_backend *self)
{
	struct x11_win *win = GP_BACKEND_PRIV(self);
	unsigned int w = self->pixmap->w;
	unsigned int h = self->pixmap->h;

	GP_DEBUG(4, "Flipping pixmap");

	if (win->resized_flag) {
		GP_DEBUG(4, "Ignoring flip, waiting for resize ack");
		return;
	}

	XLockDisplay(win->dpy);

	putimage(win, 0, 0, w - 1, h - 1);

	XFlush(win->dpy);

	process_events(win, self);

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

	struct gp_backend *self = GP_CONTAINER_OF(win, gp_backend, priv);

	switch (ev->type) {
	case Expose:
		GP_DEBUG(4, "Expose %ix%i-%ix%i %i",
		         ev->xexpose.x, ev->xexpose.y,
		         ev->xexpose.width, ev->xexpose.height,
		         ev->xexpose.count);

		if (win->resized_flag)
			break;

		/* Safety measure */
		if (ev->xexpose.x + ev->xexpose.width > (int)self->pixmap->w) {
			GP_WARN("Expose x + w > pixmap->w");
			break;
		}

		if (ev->xexpose.y + ev->xexpose.height > (int)self->pixmap->h) {
			GP_WARN("Expose y + h > pixmap->h");
			break;
		}

		/* Update the rectangle  */
		x11_update_rect(self, ev->xexpose.x, ev->xexpose.y,
		                ev->xexpose.x + ev->xexpose.width - 1,
				ev->xexpose.y + ev->xexpose.height - 1);
	break;
	case SelectionRequest:
		x11_selection_request(self, ev);
	break;
	case SelectionClear:
		x11_selection_clear(self, ev);
	break;
	case SelectionNotify:
		x11_selection_notify(self, ev);
	break;
	case ConfigureNotify:
		if (ev->xconfigure.width == (int)self->pixmap->w &&
		    ev->xconfigure.height == (int)self->pixmap->h)
			break;

		if (ev->xconfigure.width == (int)win->new_w &&
		    ev->xconfigure.height == (int)win->new_h)
			break;

		win->new_w = ev->xconfigure.width;
		win->new_h = ev->xconfigure.height;

		GP_DEBUG(4, "Configure Notify %ux%u", win->new_w, win->new_h);

		/*  Window has been resized, set flag. */
		win->resized_flag = 1;
	/* fallthrough */
	default:
		//TODO: More accurate window w and h?
		x11_input_event_put(self->event_queue, ev, win,
		                    self->pixmap->w, self->pixmap->h);
	break;
	}
}

static void process_events(struct x11_win *win, gp_backend *backend)
{
	XEvent ev;

	while (XPending(win->dpy) && !gp_ev_queue_full(backend->event_queue)) {
		XNextEvent(win->dpy, &ev);
		x11_ev(&ev);
	}
}

static enum gp_poll_event_ret x11_process_fd(gp_fd *self)
{
	gp_backend *backend = self->priv;
	struct x11_win *win = GP_BACKEND_PRIV(backend);

	XLockDisplay(win->dpy);

	process_events(win, backend);

	XUnlockDisplay(win->dpy);

	return 0;
}

static int resize_buffer(struct gp_backend *self, uint32_t w, uint32_t h)
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

static enum gp_backend_ret x11_set_attributes(struct gp_backend *self,
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
		if (win->fullscreen_flag) {
			GP_DEBUG(1, "Ignoring resize request in fullscreen");
			goto out;
		}
		GP_DEBUG(3, "Setting window size to %ux%u", w, h);
		XResizeWindow(win->dpy, win->win, w, h);
	}

	XFlush(win->dpy);

out:
	XUnlockDisplay(win->dpy);
	return 0;
}

static int x11_resize_ack(struct gp_backend *self)
{
	struct x11_win *win = GP_BACKEND_PRIV(self);
	int ret;

	XLockDisplay(win->dpy);

	GP_DEBUG(3, "Setting buffer size to %ux%u", win->new_w, win->new_h);

	ret = resize_buffer(self, win->new_w, win->new_h);

	win->resized_flag = 0;

	if (!ret) {
		gp_ev_queue_set_screen_size(self->event_queue,
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

static enum gp_pixel_type match_pixel_type(struct x11_win *win)
{
	GP_DEBUG(1, "Matching image pixel type, visual=%s depth=%u",
	         visual_class_name(win->vis->class), win->img->bits_per_pixel);

	if (win->vis->class == DirectColor || win->vis->class == TrueColor) {
		return gp_pixel_rgb_match(win->img->red_mask,
					  win->img->green_mask,
					  win->img->blue_mask, 0x0,
					  win->img->bits_per_pixel);
	}

	GP_FATAL("Unsupported visual %s", visual_class_name(win->vis->class));
	return GP_PIXEL_UNKNOWN;
}

#ifdef HAVE_X_SHM

static int create_shm_ximage(gp_backend *self, gp_size w, gp_size h)
{
	struct x11_win *win = GP_BACKEND_PRIV(self);
	enum gp_pixel_type pixel_type;

	if (XShmQueryExtension(win->dpy) == False) {
		GP_DEBUG(1, "MIT SHM Extension not supported, "
		            "falling back to XImage");
		return 1;
	}

	if (self->pixmap == NULL)
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

	win->shminfo.shmid = shmget(IPC_PRIVATE, size, 0666);

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

	gp_pixmap_init(&win->pixmap, w, h, pixel_type, win->shminfo.shmaddr, 0);
	win->pixmap.bytes_per_row = win->img->bytes_per_line;

	self->pixmap = &win->pixmap;

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

static void destroy_shm_ximage(gp_backend *self)
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

static int resize_shm_ximage(gp_backend *self, int w, int h)
{
	struct x11_win *win = GP_BACKEND_PRIV(self);
	int ret;

	GP_DEBUG(4, "Resizing XShmImage %ux%u -> %ux%u",
	         self->pixmap->w, self->pixmap->h, w, h);

	XLockDisplay(win->dpy);

	destroy_shm_ximage(self);
	ret = create_shm_ximage(self, w, h);

	XUnlockDisplay(win->dpy);

	return ret;
}

#else

static int create_shm_ximage(gp_backend GP_UNUSED(*self),
                             gp_size GP_UNUSED(w), gp_size GP_UNUSED(h))
{
	return 1;
}

static void destroy_shm_ximage(gp_backend GP_UNUSED(*self))
{
	GP_WARN("Stub called");
}

static int resize_shm_ximage(gp_backend GP_UNUSED(*self),
                             int GP_UNUSED(w), int GP_UNUSED(h))
{
	GP_WARN("Stub called");
	return 1;
}

#endif /* HAVE_X_SHM */

static int create_ximage(gp_backend *self, gp_size w, gp_size h)
{
	struct x11_win *win = GP_BACKEND_PRIV(self);
	enum gp_pixel_type pixel_type;
	int depth;

	/* Get depth similiar to the default visual depth */
	switch (win->scr_depth) {
	case 32:
	case 24:
		depth = 32;
	break;
	case 16:
		depth = 16;
	break;
	case 8:
		depth = 8;
	break;
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

	self->pixmap = gp_pixmap_alloc(w, h, pixel_type);

	if (self->pixmap == NULL) {
		GP_DEBUG(1, "Malloc failed :(");
		goto err1;
	}

	win->shm_flag = 0;
	win->img->data = (char*)self->pixmap->pixels;

	return 0;
err1:
	XDestroyImage(win->img);
err0:
	return 1;
}

static void destroy_ximage(gp_backend *self)
{
	struct x11_win *win = GP_BACKEND_PRIV(self);

	gp_pixmap_free(self->pixmap);
	win->img->data = NULL;
	XDestroyImage(win->img);
}

static int resize_ximage(gp_backend *self, int w, int h)
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

	/* Resize pixmap */
	if (gp_pixmap_resize(self->pixmap, w, h)) {
		XDestroyImage(img);
		return 1;
	}

	/* Free old image */
	win->img->data = NULL;
	XDestroyImage(win->img);

	/* Swap the pointers */
	img->data = (char*)self->pixmap->pixels;
	win->img = img;

	return 0;
}

static void window_close(gp_backend *self)
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

static void x11_exit(gp_backend *self)
{
	struct x11_win *win = GP_BACKEND_PRIV(self);

	GP_DEBUG(1, "Closing window %p", win);

	if (win == last_win)
		last_win = NULL;

	window_close(self);

	free(self);
}

static enum gp_backend_ret x11_set_attr(gp_backend *self,
                                        enum gp_backend_attr attr,
                                        const void *vals)
{
	struct x11_win *win = GP_BACKEND_PRIV(self);

	switch (attr) {
	case GP_BACKEND_ATTR_FULLSCREEN:
		return x11_win_fullscreen(win, *(const int *)vals);
	case GP_BACKEND_ATTR_TITLE:
		return x11_set_attributes(self, 0, 0, (const char *)vals);
	case GP_BACKEND_ATTR_SIZE:
		return x11_set_attributes(self, ((const int*)vals)[0], ((const int*)vals)[1], NULL);
	}

	GP_WARN("Unsupported backend attribute %i", (int) attr);
	return GP_BACKEND_NOTSUPP;
}

static int x11_set_cursor(gp_backend *self, enum gp_backend_cursors cursor)
{
	struct x11_win *win = GP_BACKEND_PRIV(self);

	switch (cursor) {
	case GP_BACKEND_CURSOR_HIDE:
		XFixesHideCursor(win->dpy, win->win);
	break;
	case GP_BACKEND_CURSOR_SHOW:
		XFixesShowCursor(win->dpy, win->win);
	break;
	case GP_BACKEND_CURSOR_ARROW:
		XDefineCursor(win->dpy, win->win, x11_conn.cursor_arrow);
	break;
	case GP_BACKEND_CURSOR_TEXT_EDIT:
		XDefineCursor(win->dpy, win->win, x11_conn.cursor_text_edit);
	break;
	case GP_BACKEND_CURSOR_CROSSHAIR:
		XDefineCursor(win->dpy, win->win, x11_conn.cursor_crosshair);
	break;
	case GP_BACKEND_CURSOR_HAND:
		XDefineCursor(win->dpy, win->win, x11_conn.cursor_hand);
	break;
	default:
		return 1;
	}

	return 0;
}

gp_backend *gp_x11_init(const char *display, int x, int y,
                        unsigned int w, unsigned int h,
                        const char *caption,
                        enum gp_x11_flags flags)
{
	gp_backend *backend;
	struct x11_win *win;
	size_t size = sizeof(gp_backend) + sizeof(struct x11_win);

	backend = malloc(size);
	if (!backend)
		return NULL;

	memset(backend, 0, size);

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

	int fd = XConnectionNumber(win->dpy);

	win->fd = (gp_fd) {
		.fd = fd,
		.event = x11_process_fd,
		.events = GP_POLLIN,
		.priv = backend,
	};

	gp_poll_add(&backend->fds, &win->fd);

	if (flags & GP_X11_FULLSCREEN)
		x11_win_fullscreen(win, GP_BACKEND_FULLSCREEN_ON);

	/* Init the event queue, once we know the window size */
	backend->event_queue = &win->ev_queue;
	gp_ev_queue_init(backend->event_queue, wreq.w, wreq.h, 0, 0);

	backend->pixmap = NULL;

	if ((flags & GP_X11_DISABLE_SHM || !x11_conn.local)
	    || create_shm_ximage(backend, wreq.w, wreq.h)) {
		if (create_ximage(backend, wreq.w, wreq.h))
			goto err1;
	}

	XFlush(win->dpy);

	backend->dpi = x11_win_get_dpi(win);

	win->resized_flag = 0;

	backend->name = "X11";
	backend->flip = x11_flip;
	backend->update_rect = x11_update_rect;
	backend->exit = x11_exit;
	backend->set_attr = x11_set_attr;
	backend->clipboard = x11_clipboard;
	backend->resize_ack = x11_resize_ack;
	backend->set_cursor = x11_set_cursor;

	return backend;
err1:
	x11_close();
	free(backend);
	return NULL;
}

#else

#include <backends/gp_backend.h>

gp_backend *gp_x11_init(const char *GP_UNUSED(display),
                        int GP_UNUSED(x), int GP_UNUSED(y),
                        unsigned int GP_UNUSED(w),
                        unsigned int GP_UNUSED(h),
                        const char *GP_UNUSED(caption))
{
	GP_FATAL("X11 support not compiled in");
	return NULL;
}

#endif /* HAVE_LIBX11 */
