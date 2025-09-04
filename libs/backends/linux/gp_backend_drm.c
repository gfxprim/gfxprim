// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2024 Cyril Hrubis <metan@ucw.cz>
 */

#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <stdint.h>

#include "../../config.h"

#include <sys/ioctl.h>
#include <sys/mman.h>
#if defined(HAVE_DRM_DRM_H)
# include <drm/drm.h>
#elif defined(HAVE_LIBDRM_DRM_H)
# include <libdrm/drm.h>
#endif

#include <core/gp_debug.h>
#include <core/gp_pixmap.h>

#include <backends/gp_backend.h>
#include <backends/gp_dpi.h>
#include <backends/gp_cursor.h>
#include <backends/gp_linux_input.h>
#include <backends/gp_linux_backlight.h>
#include <backends/gp_linux_drm.h>

#if defined(HAVE_DRM_DRM_H) || defined(HAVE_LIBDRM_DRM_H)

static int drm_open(const char *drm_path)
{
	int drm_fd;
	struct drm_get_cap cap = {
		.capability = DRM_CAP_DUMB_BUFFER,
	};

	drm_fd = open(drm_path, O_RDWR | O_NONBLOCK);
	if (drm_fd < 0) {
		GP_WARN("Failed to open '%s': %s", drm_path, strerror(errno));
		goto err0;
	}

	if (ioctl(drm_fd, DRM_IOCTL_GET_CAP, &cap)) {
		GP_WARN("Failed to get DRM capabilities, '%s' is not a DRM device?", drm_path);
		goto err1;
	}

	if (!cap.value) {
		GP_WARN("Dumb buffer not supported on '%s'!", drm_path);
		goto err1;
	}

	return drm_fd;
err1:
	close(drm_fd);
err0:
	return -1;
}

/*
 * A pixmap to draw to that is displayed on the screen, we need more than one
 * in order to switch buffers on an update.
 */
struct backend_drm_fb {
	/* frame buffer kernel pixmap handle */
	uint32_t handle;
	/* frame buffer id */
	uint32_t id;
	/* A pixel horizontal width in bytes. */
	uint32_t pitch;
	/* A frame buffer mapped buffer size. */
	uint32_t size;
	/* Pixmap width in pixels */
	uint32_t w;
	/* Pixmap height in pixels. */
	uint32_t h;
	/* Pointer to a mapped buffer */
	void *pixels;
};

struct backend_drm_priv {
	/* points to a /dev/dri/cardX */
	int drm_fd;
	/* poll callback to handle flip events */
	gp_fd poll_fd;

	/* Pixmaps and handles to feed the pixels into graphic card. */
	struct backend_drm_fb fbs[2];
	int active_fb;
	int fb_flip_in_progress:1;
	int fb_dirty:1;

	/* A backing pixmap for the application to draw into. */
	gp_pixmap fb_pixmap;

	/* input event queue */
	gp_ev_queue ev_queue;

	/* Saved state for our connector */
	struct drm_mode_crtc saved_crtc;
	int no_crtc_restore;

	/* Connector and pipeline IDs */
	uint32_t connector_id;
	uint32_t crtc_id;

	/* mode to use resolution etc. */
	struct drm_mode_modeinfo mode;

	/* Physical size of the display */
	uint32_t mm_width;
	uint32_t mm_height;

	/* Cursor max width and height */
	uint32_t max_cursor_w;
	uint32_t max_cursor_h;

	/* DRM cursor handle and size */
	uint32_t cursor_handle;
	uint32_t cursor_size;

	/* Our mapped cursor pixmap */
	gp_pixmap cursor_pixmap;

	/* Backlight driver if found */
	struct gp_linux_backlight *backlight;
};

static int mmap_fb(struct backend_drm_priv *priv, struct backend_drm_fb *fb)
{
	int fd = priv->drm_fd;

	struct drm_mode_create_dumb creq = {
		.width = priv->mode.hdisplay,
		.height = priv->mode.vdisplay,
		.bpp = 32,
	};

	GP_DEBUG(1, "Allocating fb buffer %ux%u",
	         (unsigned int)creq.width, (unsigned int)creq.height);

	if (ioctl(fd, DRM_IOCTL_MODE_CREATE_DUMB, &creq)) {
		GP_DEBUG(1, "ioctl() DRM_IOCTL_MODE_CREATE_DUMB failed: %s", strerror(errno));
		return 1;
	}

	fb->handle = creq.handle;
	fb->size = creq.size;
	fb->pitch = creq.pitch;
	fb->w = creq.width;
	fb->h = creq.height;

	struct drm_mode_destroy_dumb dreq = {
		.handle = fb->handle
	};

	struct drm_mode_fb_cmd fb_add = {
		.width = creq.width,
		.height = creq.height,
		.pitch = creq.pitch,
		.depth = 24,
		.bpp = 32,
		.handle = fb->handle
	};

	if (ioctl(fd, DRM_IOCTL_MODE_ADDFB, &fb_add)) {
		GP_DEBUG(1, "ioctl() DRM_IOCTL_MODE_ADDFB failed: %s", strerror(errno));
		goto err0;
	}

	fb->id = fb_add.fb_id;

	struct drm_mode_map_dumb mreq = {
		.handle = fb->handle
	};

	if (ioctl(fd, DRM_IOCTL_MODE_MAP_DUMB, &mreq)) {
		GP_DEBUG(1, "ioctl() DRM_IOCTL_MODE_MAP_DUMB failed: %s", strerror(errno));
		goto err1;
	}

	fb->pixels = mmap(0, fb->size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, mreq.offset);
	if (fb->pixels == MAP_FAILED) {
		GP_DEBUG(1, "Failed to mmap() framebuffer: %s", strerror(errno));
		goto err1;
	}

	memset(fb->pixels, 0, fb->size);

	GP_DEBUG(1, "Have %p framebuffer size %u", fb->pixels, (unsigned)fb->size);

	return 0;
err1:
	if (ioctl(fd, DRM_IOCTL_MODE_RMFB, &fb->id))
		GP_DEBUG(1, "ioctl() DRM_IOCTL_MODE_RMFB failed: %s", strerror(errno));
err0:
	if (ioctl(fd, DRM_IOCTL_MODE_DESTROY_DUMB, &dreq))
		GP_DEBUG(1, "ioctl() DRM_IOCTL_MODE_DESTROY_DUMB failed: %s", strerror(errno));

	return 1;
}

static void munmap_fb(struct backend_drm_priv *priv, struct backend_drm_fb *fb)
{
	int fd = priv->drm_fd;

	if (munmap(fb->pixels, fb->size))
		GP_DEBUG(1, "munmap() failed; %s", strerror(errno));

	if (ioctl(fd, DRM_IOCTL_MODE_RMFB, &fb->id))
		GP_DEBUG(1, "ioctl() DRM_IOCTL_MODE_RMFB failed: %s", strerror(errno));

	struct drm_mode_destroy_dumb dreq = {
		.handle = fb->handle
	};

	if (ioctl(fd, DRM_IOCTL_MODE_DESTROY_DUMB, &dreq))
		GP_DEBUG(1, "ioctl() DRM_IOCTL_MODE_DESTROY_DUMB failed: %s", strerror(errno));
}

static int alloc_fb_pixmap(struct backend_drm_priv *priv)
{
	struct backend_drm_fb *fb = &priv->fbs[0];

	priv->fb_pixmap.pixels = malloc(fb->h * fb->pitch * 4);

	priv->fb_pixmap.w = fb->w;
	priv->fb_pixmap.h = fb->h;
	priv->fb_pixmap.bytes_per_row = fb->pitch;
	priv->fb_pixmap.pixel_type = GP_PIXEL_xRGB8888;

	return 0;
}

static void free_fb_pixmap(struct backend_drm_priv *priv)
{
	free(priv->fb_pixmap.pixels);
}

static int map_cursor(struct backend_drm_priv *priv)
{
	int drm_fd = priv->drm_fd;

#if defined(DRM_CAP_CURSOR_WIDTH) && defined(DRM_CAP_CURSOR_HEIGHT)
	struct drm_get_cap cursor_w = {
		.capability = DRM_CAP_CURSOR_WIDTH,
	};

	struct drm_get_cap cursor_h = {
		.capability = DRM_CAP_CURSOR_HEIGHT,
	};

	if (ioctl(drm_fd, DRM_IOCTL_GET_CAP, &cursor_w)) {
		GP_DEBUG(1, "ioctl() DRM_IOCTL_GET_CAP cursor_w failed: %s",
		         strerror(errno));
		return 1;
	}

	if (ioctl(drm_fd, DRM_IOCTL_GET_CAP, &cursor_h)) {
		GP_DEBUG(1, "ioctl() DRM_IOCTL_GET_CAP cursor_h failed: %s",
			 strerror(errno));
		return 1;
	}

	priv->max_cursor_w = cursor_w.value;
	priv->max_cursor_h = cursor_h.value;
#else
	priv->max_cursor_w = 0;
	priv->max_cursor_h = 0;
#endif

	if (!priv->max_cursor_w)
		priv->max_cursor_w = 64;

	if (!priv->max_cursor_h)
		priv->max_cursor_h = 64;

	GP_DEBUG(1, "Cursor size %ux%u",
	         (unsigned int)priv->max_cursor_w,
	         (unsigned int)priv->max_cursor_h);

	struct drm_mode_create_dumb cursor_create = {
		.width = priv->max_cursor_w,
		.height = priv->max_cursor_h,
		.bpp = 32,
	};

	if (ioctl(drm_fd, DRM_IOCTL_MODE_CREATE_DUMB, &cursor_create)) {
		GP_DEBUG(1, "ioctl() DRM_IOCTL_MODE_CREATE_DUMB failed; %s",
		         strerror(errno));
		return 1;
	}

	struct drm_mode_map_dumb map_cursor = {
		.handle = cursor_create.handle
	};

	if (ioctl(drm_fd, DRM_IOCTL_MODE_MAP_DUMB, &map_cursor)) {
		GP_DEBUG(1, "ioctl() DRM_IOCTL_MODE_MAP_DUMB failed %s",
		         strerror(errno));
		goto err0;
	}

	struct drm_mode_destroy_dumb cursor_destroy  = {
		.handle = cursor_create.handle
	};

	void *cursor_pixmap = mmap(0, cursor_create.size, PROT_READ | PROT_WRITE,
	                           MAP_SHARED, drm_fd, map_cursor.offset);

	if (cursor_pixmap == MAP_FAILED) {
		GP_DEBUG(1, "mmap() failed: %s", strerror(errno));
		goto err0;
	}

	priv->cursor_handle = cursor_create.handle;
	priv->cursor_size = cursor_create.size;

	gp_pixmap_init(&priv->cursor_pixmap,
	               cursor_create.width, cursor_create.height,
	               GP_PIXEL_RGBA8888, cursor_pixmap, 0);

	memset(cursor_pixmap, 0x00, priv->cursor_size);

	struct drm_mode_cursor cursor_mode = {
		.flags = DRM_MODE_CURSOR_BO,
		.crtc_id = priv->crtc_id,
		.width = cursor_create.width,
		.height = cursor_create.height,
		.handle = priv->cursor_handle,
	};

	if (ioctl(drm_fd, DRM_IOCTL_MODE_CURSOR, &cursor_mode)) {
		GP_DEBUG(1, "ioctl() DRM_IOCTL_MODE_CURSOR failed: %s", strerror(errno));
		goto err1;
	}

	gp_cursor_render(gp_cursors_32[0], &priv->cursor_pixmap, 0xffffffff, 0xff000000, 0, 0);

	return 0;
err1:
	munmap(cursor_pixmap, cursor_create.size);
err0:
	if (ioctl(drm_fd, DRM_IOCTL_MODE_DESTROY_DUMB, &cursor_destroy))
		GP_DEBUG(1, "ioctl() DRM_IOCTL_MODE_DESTROY_DUMB failed: %s", strerror(errno));

	return 1;
}

static void munmap_cursor(struct backend_drm_priv *priv)
{
	munmap(priv->cursor_pixmap.pixels, priv->cursor_size);

	struct drm_mode_destroy_dumb cursor_destroy  = {
		.handle = priv->cursor_handle
	};

	if (ioctl(priv->drm_fd, DRM_IOCTL_MODE_DESTROY_DUMB, &cursor_destroy))
		GP_DEBUG(1, "ioctl() DRM_IOCTL_MODE_DESTROY_DUMB failed: %s", strerror(errno));
}

static void drm_move_cursor(void *priv, uint32_t x, uint32_t y)
{
	struct backend_drm_priv *drm_priv = priv;

	struct drm_mode_cursor cursor_move = {
		.flags = DRM_MODE_CURSOR_MOVE,
		.crtc_id = drm_priv->crtc_id,
		.x = x,
		.y = y,
	};

	if (ioctl(drm_priv->drm_fd, DRM_IOCTL_MODE_CURSOR, &cursor_move))
		GP_WARN("Failed to move cursor: %s", strerror(errno));
}

#define CONNECTOR_STATUS_CONNECTED 1

/*
 * Iterate through available connectors until we find something that is connected.
 */
static int init_drm(struct backend_drm_priv *priv)
{
	struct drm_mode_card_res res = {};
	int ret = 0;
	uint32_t i;
	int drm_fd = priv->drm_fd;

	if (ioctl(drm_fd, DRM_IOCTL_MODE_GETRESOURCES, &res)) {
		GP_DEBUG(1, "ioctl() DRM_IOCTL_MODE_GETRESOURCES failed: %s", strerror(errno));
		return 1;
	}

	if (res.count_connectors)
		res.connector_id_ptr = (uint64_t)(unsigned long)malloc(res.count_connectors*sizeof(uint32_t));

	res.count_fbs = 0;
	res.count_crtcs = 0;
	res.count_encoders = 0;

	if (ioctl(drm_fd, DRM_IOCTL_MODE_GETRESOURCES, &res)) {
		GP_DEBUG(1, "ioctl() DRM_IOCTL_MODE_GETRESOURCES failed: %s", strerror(errno));
		ret = 1;
		goto free;
	}

	uint32_t *connector_ids = (void*)res.connector_id_ptr;

	for (i = 0; i < res.count_connectors; i++) {
		struct drm_mode_get_connector conn = {
			.connector_id = connector_ids[i]
		};

		if (ioctl(drm_fd, DRM_IOCTL_MODE_GETCONNECTOR, &conn)) {
			GP_DEBUG(1, "ioctl() DRM_IOCTL_MODE_GETCONNECTOR failed: %s", strerror(errno));
			ret = 1;
			goto free;
		}
retry:
		if (conn.connection != CONNECTOR_STATUS_CONNECTED) {
			GP_DEBUG(2, "Connector %u is not connected", (unsigned)connector_ids[i]);
			continue;
		}

		if (!conn.count_modes) {
			GP_DEBUG(2, "No valid mode for connector %u", (unsigned)connector_ids[i]);
			continue;
		}

		conn.count_props = 0;
		conn.count_encoders = 0;

		uint32_t count_modes = conn.count_modes;

		conn.modes_ptr = (uint64_t)malloc(conn.count_modes*sizeof(struct drm_mode_modeinfo));

		if (ioctl(drm_fd, DRM_IOCTL_MODE_GETCONNECTOR, &conn)) {
			GP_DEBUG(1, "ioctl() DRM_IOCTL_MODE_GETCONNECTOR failed: %s", strerror(errno));
			ret = 1;
			free((void*)conn.modes_ptr);
			goto free;
		}

		if (conn.count_modes != count_modes) {
			GP_DEBUG(1, "conn.count_modes changed, possibly hotplug happened, retrying");
			free((void*)conn.modes_ptr);
			goto retry;
		}

		priv->connector_id = connector_ids[i];

		struct drm_mode_modeinfo *modes = (void*)conn.modes_ptr;

		GP_DEBUG(1, "Have mode %ux%u", (unsigned)modes[0].hdisplay, modes[0].vdisplay);

		priv->mm_width = conn.mm_width;
		priv->mm_height = conn.mm_height;

		priv->mode = modes[0];

		if (conn.encoder_id) {
			struct drm_mode_get_encoder enc = {
				.encoder_id = conn.encoder_id
			};

			GP_DEBUG(1, "Trying current connector encoder %u", (unsigned)conn.encoder_id);

			if (ioctl(drm_fd, DRM_IOCTL_MODE_GETENCODER, &enc)) {
				GP_DEBUG(1, "ioctl() DRM_IOCTL_MODE_GETENCODER failed: %s", strerror(errno));
				free((void*)conn.modes_ptr);
				goto free;
			}

			if (!enc.crtc_id) {
				GP_DEBUG(1, "No crtc_id for conn.encoder_id %u", (unsigned)conn.encoder_id);
				free((void*)conn.modes_ptr);
				continue;
			}

			priv->crtc_id = enc.crtc_id;

			if (mmap_fb(priv, &priv->fbs[0]))
				goto free;

			if (mmap_fb(priv, &priv->fbs[1])) {
				munmap_fb(priv, &priv->fbs[0]);
				goto free;
			}

			return 0;
		}

		GP_WARN("Haven't found any connected connector!");
	}

free:
	free((void*)res.connector_id_ptr);

	return ret;
}

static int modeset(struct backend_drm_priv *priv)
{
	if (ioctl(priv->drm_fd, DRM_IOCTL_MODE_GETCRTC, &priv->saved_crtc)) {
		GP_DEBUG(1, "ioctl() DRM_IOCTL_MODE_GETCRTC failed: %s", strerror(errno));
		priv->no_crtc_restore = 1;
	}

	struct drm_mode_crtc crtc = {
		.set_connectors_ptr = (uint64_t)(&priv->connector_id),
		.count_connectors = 1,
		.fb_id = priv->fbs[priv->active_fb].id,
		.crtc_id = priv->crtc_id,
		.mode = priv->mode,
		.mode_valid = 1,
	};

	if (ioctl(priv->drm_fd, DRM_IOCTL_MODE_SETCRTC, &crtc)) {
		GP_DEBUG(1, "ioctl() DRM_IOCTL_MODE_SETCRTC failed; %s", strerror(errno));
		return 1;
	}

	return 0;
}

static int modereset(struct backend_drm_priv *priv)
{
	if (priv->no_crtc_restore)
		return 0;

	if (ioctl(priv->drm_fd, DRM_IOCTL_MODE_SETCRTC, &priv->saved_crtc)) {
		GP_DEBUG(1, "ioctl() DRM_IOCTL_MODE_SETCRTC failed; %s", strerror(errno));
		return 1;
	}

	return 0;
}

static void backend_drm_exit(gp_backend *self)
{
	struct backend_drm_priv *priv = GP_BACKEND_PRIV(self);

	modereset(priv);
	munmap_fb(priv, &priv->fbs[0]);
	munmap_fb(priv, &priv->fbs[1]);

	free_fb_pixmap(priv);

	close(priv->drm_fd);

	gp_linux_backlight_exit(priv->backlight);

	free(self);
}

/* Copies application pixmap buffer to the inactive fb */
static void drm_copy(gp_backend *self)
{
	struct backend_drm_priv *priv = GP_BACKEND_PRIV(self);
	struct backend_drm_fb *fb = &priv->fbs[!priv->active_fb];

	memcpy(fb->pixels, self->pixmap->pixels, fb->size);
}

/* Requests the buffer flip. */
static void drm_flip(gp_backend *self)
{
	struct backend_drm_priv *priv = GP_BACKEND_PRIV(self);

	if (priv->fb_flip_in_progress) {
		priv->fb_dirty = 1;
		return;
	}

	struct backend_drm_fb *fb = &priv->fbs[!priv->active_fb];

	memcpy(fb->pixels, self->pixmap->pixels, fb->size);

	struct drm_mode_crtc_page_flip flip = {
		.fb_id = fb->id,
		.crtc_id = priv->crtc_id,
		.flags = DRM_MODE_PAGE_FLIP_EVENT,
	};

	if (ioctl(priv->drm_fd, DRM_IOCTL_MODE_PAGE_FLIP, &flip)) {
		GP_DEBUG(1, "ioctl() DRM_IOCTL_PAGE_FLIP failed; %s", strerror(errno));
		return;
	}

	priv->active_fb = !priv->active_fb;
	priv->fb_flip_in_progress = 1;
}

static void backend_drm_flip(gp_backend *self)
{
	drm_copy(self);
	drm_flip(self);
}

static void backend_drm_update_rect(gp_backend *self,
                                    gp_coord x0, gp_coord y0,
                                    gp_coord x1, gp_coord y1)
{
	(void) x0;
	(void) y0;
	(void) x1;
	(void) y1;
	backend_drm_flip(self);
}

static int backend_drm_backlight(gp_backend *self, enum gp_backend_backlight_req backlight_req)
{
	struct backend_drm_priv *priv = GP_BACKEND_PRIV(self);

	switch (backlight_req) {
	case GP_BACKEND_BACKLIGHT_INC:
		return gp_linux_backlight_inc(priv->backlight);
	case GP_BACKEND_BACKLIGHT_DEC:
		return gp_linux_backlight_dec(priv->backlight);
	case GP_BACKEND_BACKLIGHT_GET:
		return gp_linux_backlight_get(priv->backlight);
	}

	return GP_BACKEND_NOTSUPP;
}

static enum gp_backend_ret backend_drm_set_attr(gp_backend *self,
                                                enum gp_backend_attr attr,
                                                const void *vals)
{
	switch (attr) {
	case GP_BACKEND_ATTR_FULLSCREEN:
	case GP_BACKEND_ATTR_TITLE:
	case GP_BACKEND_ATTR_SIZE:
	case GP_BACKEND_ATTR_CURSOR:
		return GP_BACKEND_NOTSUPP;
	case GP_BACKEND_ATTR_BACKLIGHT:
		return backend_drm_backlight(self, *(enum gp_backend_backlight_req *)vals);
	}

	GP_WARN("Unsupported backend attribute %i", (int) attr);
	return GP_BACKEND_NOTSUPP;
}

static enum gp_poll_event_ret backend_drm_read(gp_fd *self)
{
	gp_backend *backend = self->priv;
	struct backend_drm_priv *priv = GP_BACKEND_PRIV(backend);
	char buf[1024];
	struct drm_event *e;
	ssize_t size;

	size = read(self->fd, buf, sizeof(buf));
	if (size < 0) {
		GP_WARN("Failed to read from drm fd");
		return GP_POLL_RET_OK;
	}

	e = (void*)buf;

	while (size > (ssize_t)sizeof(*e)) {
		switch (e->type) {
		case DRM_EVENT_FLIP_COMPLETE:
			priv->fb_flip_in_progress = 0;
			if (priv->fb_dirty) {
				GP_DEBUG(4, "Flipping dirty FB from DRM event handler.");
				drm_flip(backend);
				priv->fb_dirty = 0;
			}
		break;
		}

		size -= e->length;
		e = (void*)e + e->length;
	}

	return GP_POLL_RET_OK;
}

gp_backend *gp_linux_drm_init(const char *drm_path, int flags)
{
	struct backend_drm_priv *priv;
	struct gp_backend *ret;
	size_t size = sizeof(gp_backend) + sizeof(struct backend_drm_priv);

	if (flags & GP_LINUX_DRM_NO_INPUT) {
		GP_WARN("Invalid flags %i", flags);
		return NULL;
	}

	ret = malloc(size);
	if (!ret) {
		GP_WARN("Malloc failed :(");
		free(ret);
		return NULL;
	}

	memset(ret, 0, size);

	priv = GP_BACKEND_PRIV(ret);

	priv->backlight = gp_linux_backlight_init();

	if (!drm_path)
		drm_path = "/dev/dri/card0";

	priv->drm_fd = drm_open(drm_path);
	if (priv->drm_fd < 0)
		goto err0;

	priv->poll_fd.fd = priv->drm_fd;
	priv->poll_fd.event = backend_drm_read;
	priv->poll_fd.events = GP_POLLIN;
	priv->poll_fd.priv = ret;

	gp_backend_poll_add(ret, &priv->poll_fd);

	if (init_drm(priv))
		goto err1;

	if (alloc_fb_pixmap(priv)) {
		GP_WARN("Failed to allocate pixmap");
		goto err2;
	}

	if (map_cursor(priv)) {
		GP_WARN("Failed to initialize cursor!");
		goto err3;
	}

	priv->active_fb = 0;

	if (modeset(priv))
		goto err4;

	priv->active_fb = 1;

	ret->pixmap = &priv->fb_pixmap;

	ret->event_queue = &priv->ev_queue;

	gp_ev_queue_init(ret->event_queue, ret->pixmap->w, ret->pixmap->h, 0,
	                 drm_move_cursor, priv, GP_EVENT_QUEUE_LOAD_KEYMAP);
	gp_ev_queue_push(ret->event_queue, GP_EV_SYS,
	                 GP_EV_SYS_FOCUS, GP_EV_SYS_FOCUS_IN, 0);

	if (!(flags & GP_LINUX_DRM_NO_INPUT)) {
		if (gp_linux_input_hotplug_new(ret))
			goto err5;
	}

	ret->exit = backend_drm_exit;
	ret->flip = backend_drm_flip;
	ret->update_rect = backend_drm_update_rect;
	ret->set_attr = backend_drm_set_attr;

	ret->dpi = gp_dpi_from_size(ret->pixmap->w, priv->mm_width,
	                            ret->pixmap->h, priv->mm_height);

	return ret;
err5:
	modereset(priv);
err4:
	munmap_cursor(priv);
err3:
	munmap_fb(priv, &priv->fbs[0]);
	munmap_fb(priv, &priv->fbs[1]);
err2:
	free_fb_pixmap(priv);
err1:
	close(priv->drm_fd);
err0:
	gp_linux_backlight_exit(priv->backlight);
	free(ret);
	return NULL;
}

#else

gp_backend *gp_linux_drm_init(const char *drm_path, int flags)
{
	(void) drm_path;
	(void) flags;

	GP_WARN("DRM not compiled in");

	return NULL;
}

#endif /* defined(HAVE_DRM_DRM_H) || defined(HAVE_LIBDRM_DRM_H) */
