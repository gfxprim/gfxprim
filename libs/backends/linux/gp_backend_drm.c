// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2022 Cyril Hrubis <metan@ucw.cz>
 */

#include <fcntl.h>
#include <string.h>
#include <errno.h>

#include "../../config.h"

#include <sys/ioctl.h>
#include <sys/mman.h>
#ifdef HAVE_DRM_DRM_H
# include <drm/drm.h>
#elif HAVE_LIBDRM_DRM_H
# include <libdrm/drm.h>
#endif

#include <backends/gp_backend.h>
#include <backends/gp_linux_drm.h>
#include <core/gp_debug.h>
#include <core/gp_pixmap.h>

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

struct backend_drm_priv {
	/* points to a /dev/dri/cardX */
	int drm_fd;

	/* framebuffer pixmap */
	gp_pixmap pixmap;

	/* Saved state for our connector */
	struct drm_mode_crtc saved_crtc;
	int no_crtc_restore;

	uint32_t connector_id;
	uint32_t crtc_id;

	/* mode to use resolution etc. */
	struct drm_mode_modeinfo mode;

	/* frame buffer info */
	uint32_t fb_handle;
	uint32_t fb_id;
	uint32_t fb_size;
};

static int mmap_fb(struct backend_drm_priv *priv)
{
	int fd = priv->drm_fd;

	struct drm_mode_create_dumb creq = {
		.width = priv->pixmap.w,
		.height = priv->pixmap.h,
		.bpp = 32,
	};

	if (ioctl(fd, DRM_IOCTL_MODE_CREATE_DUMB, &creq)) {
		GP_DEBUG(1, "ioctl() DRM_IOCTL_MODE_CREATE_DUMB failed: %s", strerror(errno));
		return 1;
	}

	priv->fb_handle = creq.handle;
	priv->fb_size = creq.size;

	priv->pixmap.bpp = 32;
	priv->pixmap.bytes_per_row = creq.pitch;
	priv->pixmap.pixel_type = GP_PIXEL_xRGB8888;

	struct drm_mode_destroy_dumb dreq = {
		.handle = priv->fb_handle
	};

	struct drm_mode_fb_cmd fb = {
		.width = priv->pixmap.w,
		.height = priv->pixmap.h,
		.pitch = priv->pixmap.bytes_per_row,
		.bpp = priv->pixmap.bpp,
		.depth = 24,
		.handle = priv->fb_handle
	};

	if (ioctl(fd, DRM_IOCTL_MODE_ADDFB, &fb)) {
		GP_DEBUG(1, "ioctl() DRM_IOCTL_MODE_ADDFB failed: %s", strerror(errno));
		goto err0;
	}

	priv->fb_id = fb.fb_id;

	struct drm_mode_map_dumb mreq = {
		.handle = priv->fb_handle
	};

	if (ioctl(fd, DRM_IOCTL_MODE_MAP_DUMB, &mreq)) {
		GP_DEBUG(1, "ioctl() DRM_IOCTL_MODE_MAP_DUMB failed: %s", strerror(errno));
		goto err1;
	}

	priv->pixmap.pixels = mmap(0, priv->fb_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, mreq.offset);
	if (priv->pixmap.pixels == MAP_FAILED) {
		GP_DEBUG(1, "Failed to mmap() framebuffer: %s", strerror(errno));
		goto err1;
	}

	memset(priv->pixmap.pixels, 0, priv->fb_size);

	GP_DEBUG(1, "Have %p framebuffer size %u", priv->pixmap.pixels, (unsigned)priv->fb_size);

	return 0;
err1:
	if (ioctl(fd, DRM_IOCTL_MODE_RMFB, &priv->fb_id))
		GP_DEBUG(1, "ioctl() DRM_IOCTL_MODE_RMFB failed: %s", strerror(errno));
err0:

	if (ioctl(fd, DRM_IOCTL_MODE_DESTROY_DUMB, &dreq))
		GP_DEBUG(1, "ioctl() DRM_IOCTL_MODE_DESTROY_DUMB failed: %s", strerror(errno));

	return 1;
}

static void munmap_fb(struct backend_drm_priv *priv)
{
	int fd = priv->drm_fd;

	if (munmap(priv->pixmap.pixels, priv->fb_size))
		GP_DEBUG(1, "munmap() failed; %s", strerror(errno));

	if (ioctl(fd, DRM_IOCTL_MODE_RMFB, &priv->fb_id))
		GP_DEBUG(1, "ioctl() DRM_IOCTL_MODE_RMFB failed: %s", strerror(errno));

	struct drm_mode_destroy_dumb dreq = {
		.handle = priv->fb_handle
	};

	if (ioctl(fd, DRM_IOCTL_MODE_DESTROY_DUMB, &dreq))
		GP_DEBUG(1, "ioctl() DRM_IOCTL_MODE_DESTROY_DUMB failed: %s", strerror(errno));
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

		priv->pixmap.w = modes[0].hdisplay;
		priv->pixmap.h = modes[0].vdisplay;

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

			if (!mmap_fb(priv))
				goto free;
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
		.fb_id = priv->fb_id,
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
	munmap_fb(priv);
	close(priv->drm_fd);
}

gp_backend *gp_linux_drm_init(const char *drm_path, int flags)
{
	struct backend_drm_priv *priv;
	struct gp_backend *ret;
	size_t size = sizeof(gp_backend) + sizeof(struct backend_drm_priv);

	if (flags) {
		GP_WARN("Invalid flags %i", flags);
		return NULL;
	}

	if (!drm_path)
		drm_path = "/dev/dri/card0";

	ret = malloc(size);
	if (!ret) {
		GP_WARN("Malloc failed :(");
		return NULL;
	}

	memset(ret, 0, size);

	priv = GP_BACKEND_PRIV(ret);

	priv->drm_fd = drm_open(drm_path);
	if (priv->drm_fd < 0)
		goto err0;

	if (init_drm(priv))
		goto err1;

	if (modeset(priv))
		goto err2;

	ret->exit = backend_drm_exit;
	ret->pixmap = &priv->pixmap;

	return ret;
err2:
	munmap_fb(priv);
err1:
	close(priv->drm_fd);
err0:
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
