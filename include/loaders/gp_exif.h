// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2009-2014 Cyril Hrubis <metan@ucw.cz>
 */

 /*

   EXIF parser.

  */

#ifndef LOADERS_GP_EXIF_H
#define LOADERS_GP_EXIF_H

#include <loaders/gp_data_storage.h>
#include <loaders/gp_loader.h>

/*
 * Reads EXIF from an readable I/O stream and stores the values into storage.
 */
int gp_read_exif(gp_io *io, gp_storage *storage);

/*
 * Looks for EXIF file signature. Returns non-zero if found.
 */
int gp_match_exif(const void *buf);

/*
 * Defines position to 0,0 coordinate.
 */
enum GP_EXIF_ORIENTATION {
	GP_EXIF_UPPER_LEFT = 1,
	GP_EXIF_LOWER_RIGHT = 3,
	GP_EXIF_UPPER_RIGHT = 6,
	GP_EXIF_LOWER_LEFT = 8,
};

#endif /* LOADERS_GP_EXIF_H */
