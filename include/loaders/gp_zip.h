// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2009-2013 Cyril Hrubis <metan@ucw.cz>
 */

 /*

   Zip container, could be used to load images from cbz or from zip files.

  */

#ifndef LOADERS_GP_ZIP_H
#define LOADERS_GP_ZIP_H

#include <core/gp_types.h>
#include <core/gp_progress_callback.h>

#include <loaders/gp_container.h>

gp_container *gp_open_zip(const char *path);

int gp_match_zip(const char *buf);

#endif /* LOADERS_GP_ZIP_H */
