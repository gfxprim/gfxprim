// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2009-2024 Cyril Hrubis <metan@ucw.cz>
 */

 /**
  * @file gp_exif.h
  * @brief EXIF parser.
  */

#ifndef LOADERS_GP_EXIF_H
#define LOADERS_GP_EXIF_H

#include <core/gp_gamma_correction.h>
#include <loaders/gp_data_storage.h>
#include <loaders/gp_loader.h>

/**
 * @brief Reads EXIF from a readable I/O stream and stores the values into storage.
 *
 * @param io An input I/O stream.
 * @param storage A data storage for exif metadata, may be NULL if not needed.
 * @param corr_desc A correction tables parsed from the exif.
 *
 * @return Zero on success, non-zero otherwise.
 */
int gp_read_exif(gp_io *io, gp_storage *storage, gp_correction_desc *corr_desc);

/**
 * @brief Defines position to 0,0 coordinate.
 */
enum GP_EXIF_ORIENTATION {
	/** @brief Upper left. */
	GP_EXIF_UPPER_LEFT = 1,
	/** @brief Lower right. */
	GP_EXIF_LOWER_RIGHT = 3,
	/** @brief Upper right. */
	GP_EXIF_UPPER_RIGHT = 6,
	/** @brief Lower left. */
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
