// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2009-2026 Cyril Hrubis <metan@ucw.cz>
 */

 /**
  * @brief Zip container, could be used to load images from cbz or from zip
  * files.
  */

#ifndef LOADERS_GP_ZIP_H
#define LOADERS_GP_ZIP_H

#include <loaders/gp_container.h>

extern const gp_container_ops gp_zip_ops;

/**
 * @brief Opens a zip container.
 *
 * @param path A path to a zip file.
 * @return Newly allocated and initialized zip container.
 */
gp_container *gp_open_zip(const char *path);

/**
 * @brief Creates a zip container from an I/O.
 *
 * @warning No check are done, the caller is supposed to use gp_match_zip() to
 *          check for the zip signature.
 *
 * @return Newly allocated and initialized zip container.
 */
gp_container *gp_init_zip(gp_io *io);

/**
 * @brief buf A buffer with a first 32 bytes of a file.
 *
 * @return Non-zero if a zip signature was found.
 */
int gp_match_zip(const void *buf);

#endif /* LOADERS_GP_ZIP_H */
