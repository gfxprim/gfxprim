//SPDX-License-Identifier: LGPL-2.0-or-later
/*

   Copyright (c) 2019-2024 Cyril Hrubis <metan@ucw.cz>

 */

/**
 * @brief A SHM pixmap.
 * @file gp_proxy_shm.h
 */
#ifndef GP_PROXY_SHM_H
#define GP_PROXY_SHM_H

#include <backends/gp_proxy_proto.h>
#include <core/gp_pixmap.h>

/**
 * @brief A SHM pixmap.
 */
typedef struct gp_proxy_shm {
	/** @brief A file descriptor for the SHM file. */
	int fd;
	/** @brief A SHM segment size. */
	size_t size;
	/** @brief A pixmap with the SHM segment as a pixels. */
	gp_pixmap pixmap;
	/** @brief A path to the SHM segment. */
	struct gp_proxy_path path;
} gp_proxy_shm;

/**
 * @brief Creates an SHM pixmap.
 *
 * This is called by the server to initialize a SHM pixmap for the application
 * to render to. The SHM segment size is rounded to PAGE_SIZE.
 *
 * @param path in the /dev/shm/ filesystem, 64 bytes at max.
 * @param w Image width.
 * @param h Image height.
 * @param type Image pixel type.
 *
 * @return Newly created SHM pixmap.
 */
gp_proxy_shm *gp_proxy_shm_init(const char *path, gp_size w, gp_size h, gp_pixel_type type);

/**
 * @brief Resizes a SHM pixmap.
 *
 * Resizes SHM image. All applications have unmap the buffer before this call
 * is called and have to remap the shm before they attempt to use it again.
 *
 * @param self A SHM pixmap.
 * @param w New width.
 * @parm h New height.
 *
 * @return 0 if shm buffer haven't needed to be resized, only pixmap size has been changed
 *         1 if buffer was resized
 *         -1 on failure
 */
int gp_proxy_shm_resize(gp_proxy_shm *self, gp_size w, gp_size h);

/**
 * @brief Removes a SHM pixmap.
 *
 * @param self A SHM pixmap to be removed and deallocated.
 */
void gp_proxy_shm_exit(gp_proxy_shm *self);

#endif /* GP_PROXY_SHM_H */
