// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2009-2019 Cyril Hrubis <metan@ucw.cz>
 */

#include "../../config.h"

#include <core/gp_debug.h>
#include "core/gp_common.h"
#include <backends/gp_backend.h>

#ifdef HAVE_LIBXCB

#include <string.h>
#include <sys/shm.h>
#include <xcb/xcb.h>
#include <xcb/shm.h>

#include <core/gp_pixmap.h>

#include "gp_xcb_con.h"
#include "gp_xcb_input.h"

struct win {
	xcb_screen_t *scr;
	xcb_window_t win;
	xcb_gcontext_t gc;
	xcb_pixmap_t pixmap;
	xcb_shm_seg_t shmseg;

	int new_w;
	int new_h;

	int resized:1;
	int fullscreen:1;
};

static void x_exit(gp_backend *self)
{
	x_close();

	free(self);
}

static void *attach_shm_pixmap(struct gp_backend *self, xcb_connection_t *c,
                               gp_pixel_type pixel_type, int w, int h)
{
	struct win *win = GP_BACKEND_PRIV(self);

	GP_DEBUG(3, "Attaching SHM pixmap %ix%i", w, h);

	uint64_t size = gp_pixel_size(pixel_type) * w * h;

	int shm_id = shmget(IPC_PRIVATE, size, IPC_CREAT | 0777);

	if (shm_id == -1) {
		GP_WARN("shmget() failed");
		return NULL;
	}

	void *shm_addr = shmat(shm_id, 0, 0);

	if (shm_addr == (void*)-1) {
		GP_WARN("shmat() failed");
		goto err1;
	}

	xcb_void_cookie_t cookie;
	xcb_generic_error_t *err;

	cookie = xcb_shm_attach_checked(c, win->shmseg, shm_id, 0);
	err = xcb_request_check(c, cookie);

	shmctl(shm_id, IPC_RMID, 0);

	if (err) {
		GP_WARN("xcb_shm_attach() failed");
		free(err);
		return NULL;
	}

	return shm_addr;
err1:
	shmctl(shm_id, IPC_RMID, 0);
	return NULL;
}

static int detach_shm_pixmap(struct gp_backend *self, xcb_connection_t *c)
{
	struct win *win = GP_BACKEND_PRIV(self);

	xcb_void_cookie_t cookie;
	xcb_generic_error_t *err;

	GP_DEBUG(3, "Detaching SHM pixmap");

	cookie = xcb_shm_detach_checked(c, win->shmseg);
	err = xcb_request_check(c, cookie);

	if (err) {
		GP_WARN("xcb_shm_detach() failed");
		free(err);
		return 1;
	}

	return 0;
}

static int resize_shm_pixmap(struct gp_backend *self, xcb_connection_t *c)
{
	struct win *win = GP_BACKEND_PRIV(self);
	void *shm_addr;

	if (detach_shm_pixmap(self, c))
		return 1;

	shm_addr = attach_shm_pixmap(self, c, self->pixmap->pixel_type,
	                             win->new_w, win->new_h);

	if (!shm_addr)
		return 1;

	gp_pixmap_init(self->pixmap, win->new_w, win->new_h,
	               self->pixmap->pixel_type, shm_addr, 0);

	return 0;
}

static int resize_pixmap(gp_backend *self, xcb_connection_t *c)
{
	struct win *win = GP_BACKEND_PRIV(self);

	/* Resize backing store */
	xcb_free_pixmap(c, win->pixmap);
	xcb_create_pixmap(c, win->scr->root_depth, win->pixmap,
	                  win->win, win->new_w, win->new_h);

	/* Resize backend pixmap */
	return gp_pixmap_resize(self->pixmap, win->new_w, win->new_h);
}

static void put_image_strip(xcb_connection_t *c, struct win *win, gp_pixmap *pixmap,
                            int y, int h)
{
	xcb_put_image(c, XCB_IMAGE_FORMAT_Z_PIXMAP, win->pixmap,
		      win->gc, pixmap->w, h, 0, y, 0,
		      win->scr->root_depth, pixmap->bytes_per_row * h,
		      pixmap->pixels +
		      pixmap->bytes_per_row * y);
}

/*
 * If we are putting the image on remote machine it's better to send updates as
 * stripes since that way user will see the image being loaded rather than
 * black window until everything is uploaded.
 */
static void put_image_remote(xcb_connection_t *c, struct win *win, gp_pixmap *pixmap,
                             int x, int y, int w, int h)
{
	int i;
	int steps = GP_MAX(1, h / 200 * (int)pixmap->w / 200);
	int strip_size = h / steps;

	for (i = 0; i < steps - 1; i++) {
		int cy = y + i * strip_size;
		put_image_strip(c, win, pixmap, cy, strip_size);
		xcb_copy_area(c, win->pixmap, win->win, win->gc, x, cy, x, cy, w, strip_size);
	}

	int hr = h - strip_size * (steps - 1);
	int cy = y + (steps - 1) * strip_size;

	put_image_strip(c, win, pixmap, cy, hr);
	xcb_copy_area(c, win->pixmap, win->win, win->gc, x, cy, x, cy, w, hr);
}

static void put_image(gp_backend *self, int x, int y, int w, int h)
{
	struct win *win = GP_BACKEND_PRIV(self);

	if (!x_con.local) {
		put_image_remote(x_con.c, win, self->pixmap, x, y, w, h);
	} else {
		put_image_strip(x_con.c, win, self->pixmap, y, h);
		xcb_copy_area(x_con.c, win->pixmap, win->win, win->gc, x, y, x, y, w, h);
	}

	xcb_flush(x_con.c);
}

static void put_shm_image(gp_backend *self, int x, int y, int w, int h)
{
	struct win *win = GP_BACKEND_PRIV(self);
	xcb_shm_put_image(x_con.c, win->win, win->gc, self->pixmap->w, self->pixmap->h, x, y, w, h, x, y,
	                  win->scr->root_depth, XCB_IMAGE_FORMAT_Z_PIXMAP, 1, win->shmseg, 0);
	xcb_flush(x_con.c);
}

static void x_update_rect(gp_backend *self, gp_coord x0, gp_coord y0,
                             gp_coord x1, gp_coord y1)
{
	struct win *win = GP_BACKEND_PRIV(self);

	GP_DEBUG(4, "Updating rect %ix%i-%ix%i", x0, y0, x1, y1);

	if (win->resized) {
		GP_DEBUG(4, "Ignoring update rect, waiting for resize ack");
		return;
	}

	gp_size w = x1-x0+1;
	gp_size h = y1-y0+1;

	if (x_con.shm_support)
		put_shm_image(self, x0, y0, w, h);
	else
		put_image(self, x0, y0, w, h);
}

static void x_flip(gp_backend *self)
{
	x_update_rect(self, 0, 0, self->pixmap->w - 1, self->pixmap->h - 1);
}

static int x_resize_ack(struct gp_backend *self)
{
	struct win *win = GP_BACKEND_PRIV(self);

	GP_DEBUG(3, "Setting buffer size to %ux%u", win->new_w, win->new_h);

        int ret;

	if (x_con.shm_support)
		ret = resize_shm_pixmap(self, x_con.c);
	else
		ret = resize_pixmap(self, x_con.c);

	win->resized = 0;

	if (!ret) {
		gp_event_queue_set_screen_size(&self->event_queue,
		                               win->new_w, win->new_h);
	}

	GP_DEBUG(3, "Done");

	return ret;
}


static void x_ev(gp_backend *self, xcb_generic_event_t *ev)
{
	struct win *win = GP_BACKEND_PRIV(self);

	switch (ev->response_type & ~0x80) {
	case XCB_EXPOSE: {
		xcb_expose_event_t *eev = (xcb_expose_event_t *)ev;

		GP_DEBUG(2, "Expose %ix%i-%ix%i %i",
		         eev->x, eev->y, eev->width, eev->height, eev->count);

		/* Safety measure */
		if (eev->x + eev->width > (int)self->pixmap->w) {
			GP_WARN("Expose x + w > pixmap->w");
			break;
		}

		if (eev->y + eev->height > (int)self->pixmap->h) {
			GP_WARN("Expose y + h > pixmap->h");
			break;
		}

		if (x_con.shm_support) {
			put_shm_image(self, eev->x, eev->y, eev->width, eev->height);
		} else {
			xcb_copy_area(x_con.c, win->pixmap, win->win, win->gc,
			              eev->x, eev->y,
			              eev->x, eev->y,
			              eev->width, eev->height);
		}
		xcb_flush(x_con.c);
	} break;
	case XCB_CONFIGURE_NOTIFY: {
		xcb_configure_notify_event_t *cev = (xcb_configure_notify_event_t *)ev;

		if (cev->width == (int)self->pixmap->w &&
		    cev->height == (int)self->pixmap->h)
			break;

		if (cev->width == (int)win->new_w &&
		    cev->height == (int)win->new_h)
			break;

		win->new_w = cev->width;
		win->new_h = cev->height;

		GP_DEBUG(2, "Configure Notify %ux%u", cev->width, cev->height);

		/*  Window has been resized, set flag. */
		win->resized = 1;
	} /* fallthru */
	default:
		xcb_input_event_put(&self->event_queue, ev,
		                    self->pixmap->w, self->pixmap->h);
	}
}

static void x_poll(gp_backend *self)
{
	xcb_generic_event_t *ev;

	while ((ev = xcb_poll_for_event(x_con.c))) {
		x_ev(self, ev);
		free(ev);
	}
}

//TODO: Generic wait for backends!
#include <poll.h>

static void x_wait(gp_backend *self)
{
	struct pollfd fd = {.fd = self->fd, .events = POLLIN, .revents = 0};
	poll(&fd, 1, -1);
	x_poll(self);
}

static xcb_visualtype_t *visual_by_id(struct win *win, int *depth)
{
	xcb_depth_iterator_t depth_iter = xcb_screen_allowed_depths_iterator(win->scr);

	for (; depth_iter.rem; xcb_depth_next(&depth_iter)) {
		xcb_visualtype_iterator_t visual_iter = xcb_depth_visuals_iterator(depth_iter.data);

		*depth = depth_iter.data->depth;

		for (; visual_iter.rem; xcb_visualtype_next(&visual_iter)) {
			if (win->scr->root_visual == visual_iter.data->visual_id)
				return visual_iter.data;
		}
	}

	GP_WARN("Couldn't find window root_visual %i", win->scr->root_visual);

	return NULL;
}

static int bpp_by_depth(xcb_connection_t *c, int depth)
{
	const xcb_setup_t *setup = xcb_get_setup(c);
	xcb_format_iterator_t fmt_iter = xcb_setup_pixmap_formats_iterator(setup);

	for (; fmt_iter.rem; xcb_format_next(&fmt_iter)) {
		if (fmt_iter.data->depth == depth)
			return fmt_iter.data->bits_per_pixel;
	}

	return 0;
}

static int create_shm_backing_pixmap(struct gp_backend *self, xcb_connection_t *c,
                                     gp_pixel_type pixel_type, int w, int h)
{
	struct win *win = GP_BACKEND_PRIV(self);
	void *shm_addr;

	self->pixmap = malloc(sizeof(gp_pixmap));

	win->shmseg = xcb_generate_id(c);

	shm_addr = attach_shm_pixmap(self, c, pixel_type, w, h);
	if (!shm_addr) {
		free(self->pixmap);
		return 1;
	}

	gp_pixmap_init(self->pixmap, w, h, pixel_type, shm_addr, 0);

	return 0;
}

static int create_backing_pixmap(struct gp_backend *self, xcb_connection_t *c,
                                 gp_pixel_type pixel_type, int w, int h)
{
	struct win *win = GP_BACKEND_PRIV(self);

	win->pixmap = xcb_generate_id(c);

	if (x_con.shm_support) {
		create_shm_backing_pixmap(self, c, pixel_type, w, h);
	} else {
		xcb_create_pixmap(c, win->scr->root_depth, win->pixmap, win->win, w, h);
		self->pixmap = gp_pixmap_alloc(w, h, pixel_type);
	}

	if (!self->pixmap)
		return 1;

	return 0;
}

static void set_title(xcb_connection_t *c, xcb_window_t win, const char *str)
{
	xcb_change_property(c, XCB_PROP_MODE_REPLACE, win,
	                    XCB_ATOM_WM_NAME, XCB_ATOM_STRING,
	                    8, strlen(str), str);
}

static int x_set_size(gp_backend *self, const uint32_t *size)
{
	struct win *win = GP_BACKEND_PRIV(self);
	xcb_connection_t *c = x_con.c;

	if (size[0] == 0 || size[1] == 0)
		return 1;

	if (win->fullscreen) {
		GP_DEBUG(1, "Ignoring resize request in fullscreen");
		return 1;
	}

	GP_DEBUG(3, "Setting window size to %ux%u", size[0], size[1]);

	xcb_configure_window(c, win->win,
		             XCB_CONFIG_WINDOW_WIDTH | XCB_CONFIG_WINDOW_HEIGHT,
		             size);
	xcb_flush(c);
	return 0;
}

static int x_set_title(gp_backend *self, const char *title)
{
	struct win *win = GP_BACKEND_PRIV(self);
	xcb_connection_t *c = x_con.c;

	GP_DEBUG(3, "Setting window title to '%s'", title);

	set_title(c, win->win, title);
	xcb_flush(c);
	return 0;
}

static int x_set_fullscreen(gp_backend *self, const int *val)
{
	struct win *win = GP_BACKEND_PRIV(self);
	xcb_connection_t *c = x_con.c;

	if (!x_con.state_supported || !x_con.fullscreen_supported)
		return 1;

	GP_DEBUG(3, "Changing fullscreen to %i", *val);

	xcb_client_message_event_t ev = {
		.response_type = XCB_CLIENT_MESSAGE,
		.format = 32,
		.window = win->win,
		.type = x_con.net_wm_state,
		.data = {
			.data32 = {
				*val,
				x_con.net_wm_state_fullscreen,
				0,
				1,
			}
		}
	};

	xcb_send_event(c, 0, win->scr->root,
	               XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY |
	               XCB_EVENT_MASK_SUBSTRUCTURE_REDIRECT,
	               (void*)&ev);

//	xcb_change_property(c, XCB_PROP_MODE_REPLACE, win->win, x_con.net_wm_state,
//			    XCB_ATOM, 32, 1, &x_con.net_wm_state_fullscreen);
	xcb_flush(c);

	return 0;
}

static int x_set_attr(gp_backend *self, enum gp_backend_attrs attr,
                      const void *vals)
{
	switch (attr) {
	case GP_BACKEND_SIZE:
		return x_set_size(self, vals);
	case GP_BACKEND_TITLE:
		return x_set_title(self, vals);
	case GP_BACKEND_FULLSCREEN:
		return x_set_fullscreen(self, vals);
	}

	GP_WARN("Invalid backend attribute type %i", attr);

	return 1;
}

static int create_window(struct gp_backend *self, struct win *win,
                         int x, int y, int w, int h,
                         const char *caption)
{
	xcb_connection_t *c = x_con.c;

	win->scr = xcb_setup_roots_iterator(xcb_get_setup(c)).data;

	/* Create window */
	win->win = xcb_generate_id(c);

	uint32_t mask_val[] = {win->scr->black_pixel,
	                       XCB_EVENT_MASK_EXPOSURE |
	                       XCB_EVENT_MASK_KEY_PRESS |
			       XCB_EVENT_MASK_KEY_RELEASE |
	                       XCB_EVENT_MASK_BUTTON_PRESS |
			       XCB_EVENT_MASK_BUTTON_RELEASE |
			       XCB_EVENT_MASK_POINTER_MOTION |
			       XCB_EVENT_MASK_STRUCTURE_NOTIFY};

	xcb_create_window(c,
	                  XCB_COPY_FROM_PARENT,
			  win->win,
	                  win->scr->root,
	                  x, y, w, h,
			  0,
	                  XCB_WINDOW_CLASS_INPUT_OUTPUT,
			  win->scr->root_visual,
	                  XCB_CW_BACK_PIXEL | XCB_CW_EVENT_MASK,
			  &mask_val);

	if (x_con.delete_window_supported) {
		xcb_change_property(c, XCB_PROP_MODE_REPLACE, win->win,
				    x_con.wm_protocols, 4, 32, 1, &x_con.wm_delete_window);
	}

	set_title(c, win->win, caption);

	gp_event_queue_init(&self->event_queue, w, h, 0);

	/* Get pixel format */
	int depth;
	xcb_visualtype_t *visual = visual_by_id(win, &depth);

	if (!visual)
		return 1;

	if (visual->_class != XCB_VISUAL_CLASS_TRUE_COLOR &&
	    visual->_class != XCB_VISUAL_CLASS_DIRECT_COLOR) {
		GP_WARN("Unsupported visual->_class %i", visual->_class);
		return 1;
	};
	gp_pixel pixel_type = gp_pixel_rgb_match(visual->red_mask, visual->green_mask, visual->blue_mask, 0, bpp_by_depth(c, depth));

	if (pixel_type == GP_PIXEL_UNKNOWN) {
		GP_DEBUG(1, "Unknown pixel type");
		return 1;
	}

	create_backing_pixmap(self, c, pixel_type, w, h);

	/* Create GC */
	uint32_t values[] = {win->scr->black_pixel, win->scr->white_pixel};
	win->gc = xcb_generate_id(x_con.c);
	xcb_create_gc(x_con.c, win->gc, win->win, XCB_GC_FOREGROUND | XCB_GC_BACKGROUND, values);

	/* Finally show the window */
	xcb_map_window(c, win->win);
	xcb_flush(c);

	return 0;
}

gp_backend *gp_xcb_init(const char *display, int x, int y, int w, int h,
                        const char *caption)
{
	gp_backend *backend;
	struct win *win;
	size_t size = sizeof(gp_backend) + sizeof(struct win);

	backend = malloc(size);
	if (!backend)
		return NULL;

	memset(backend, 0, size);

	if (!x_connect(display))
		goto err;

	win = GP_BACKEND_PRIV(backend);

	if (create_window(backend, win, x, y, w, h, caption))
		goto err;

	backend->name = "XCB";
	backend->flip = x_flip;
	backend->update_rect = x_update_rect;
	backend->exit = x_exit;
	backend->poll = x_poll;
	backend->wait = x_wait;
	backend->set_attr = x_set_attr;
	backend->resize_ack = x_resize_ack;
	backend->fd = xcb_get_file_descriptor(x_con.c);

	return backend;
err:
	free(backend);
	return NULL;
}

#else

gp_backend *gp_xcb_init(const char *GP_UNUSED(display),
                        int GP_UNUSED(x), int GP_UNUSED(y),
                        unsigned int GP_UNUSED(w),
                        unsigned int GP_UNUSED(h),
                        const char *GP_UNUSED(caption))
{
	GP_FATAL("XCB support not compiled in");
	return NULL;
}

#endif /* HAVE_LIBXCB */
