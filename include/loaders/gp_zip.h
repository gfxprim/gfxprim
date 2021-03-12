// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2009-2021 Cyril Hrubis <metan@ucw.cz>
 */

 /*

   Zip container, could be used to load images from cbz or from zip files.

  */

#ifndef LOADERS_GP_ZIP_H
#define LOADERS_GP_ZIP_H

#include <loaders/gp_container.h>

extern const gp_container_ops gp_zip_ops;

gp_container *gp_open_zip(const char *path);

/*
 * Creates a new ZIP container from an io. No check are done, the caller is
 * supposed to use gp_match_zip() to check for the zip signature.
 */
gp_container *gp_init_zip(gp_io *io);

int gp_match_zip(const void *buf);

#endif /* LOADERS_GP_ZIP_H */
