//SPDX-License-Identifier: LGPL-2.0-or-later
/*

   Copyright (c) 2019-2020 Cyril Hrubis <metan@ucw.cz>

 */

#ifndef GP_PROXY_SHM_H
#define GP_PROXY_SHM_H

#include <backends/gp_proxy_proto.h>
#include <core/gp_pixmap.h>

struct gp_proxy_shm {
	int fd;
	size_t size;
	gp_pixmap pixmap;
	struct gp_proxy_path path;
};

/*
 * Creates an SHM pixmap.
 *
 * @path in the /dev/shm/ filesystem, 64 bytes at max.
 */
struct gp_proxy_shm *gp_proxy_shm_init(const char *path, gp_size w, gp_size h, gp_pixel_type type);

/*
 * Resizes SHM image, all clients has to avoid working with the buffer once
 * this call is called and have to remap the shm before they attempt to use it again.
 *
 * @returns 0 if shm buffer haven't needed to be resized, only pixmap size has been changed
 *          1 if buffer was resized
 *          -1 on failure
 */
int gp_proxy_shm_resize(struct gp_proxy_shm *self, gp_size w, gp_size h);

void gp_proxy_shm_exit(struct gp_proxy_shm *self);

#endif /* GP_PROXY_SHM_H */
