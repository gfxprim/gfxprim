// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2009-2024 Cyril Hrubis <metan@ucw.cz>
 */

 /*

   EXIF parser.

  */

#ifndef LOADERS_GP_EXIF_H
#define LOADERS_GP_EXIF_H

#include <core/gp_gamma_correction.h>
#include <loaders/gp_data_storage.h>
#include <loaders/gp_loader.h>

/**
 * @brief Reads EXIF from a readable I/O stream and stores the values into storage.
 *
 * @io An input I/O stream.
 * @storage A data storage for exif metadata, may be NULL if not needed.
 * @corr_desc A correction parsed from the exif.
 */
int gp_read_exif(gp_io *io, gp_storage *storage, gp_correction_desc *corr_desc);

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

/**
 * @brief Color space values.
 */
enum gp_exif_color_space {
	/** @brief sRGB color space */
	GP_EXIF_COLOR_SPACE_SRGB = 1,
	/** @brief Adobe RGB is Undefined + Gamma = 22/10 */
	GP_EXIF_COLOR_SPACE_UNDEFINED = 0xffff,
};

#endif /* LOADERS_GP_EXIF_H */
