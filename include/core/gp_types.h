// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2009-2024 Cyril Hrubis <metan@ucw.cz>
 */

/**
 * @file gp_types.h
 * @brief A common types.
 */

#ifndef CORE_GP_TYPES_H
#define CORE_GP_TYPES_H

#include <stdint.h>

/**
 * @brief Integer type for coordinates i.e. x, y, ...
 */
typedef int gp_coord;

/**
 * @brief Integer type for sizes i.e. w, h, ...
 */
typedef unsigned int gp_size;

typedef int gp_ssize;

/**
 * @brief Pixel integer value.
 *
 * All channels are packed into the value accordingly to the enum #gp_pixel_type
 */
typedef uint32_t gp_pixel;

/* Pixel type description */
typedef struct gp_pixel_type_desc gp_pixel_type_desc;

/* Bitmap image */
typedef struct gp_pixmap gp_pixmap;

/* Progress callback */
typedef struct gp_progress_cb gp_progress_cb;

/**
 * @brief Rotation and mirroring flags.
 */
typedef enum gp_symmetry {
	/** @brief Rotate 90 degrees clockwise. */
	GP_ROTATE_90 = 0,
	/** @brief Rotate clockwise. Alias for GP_ROTATE_90 */
	GP_ROTATE_CW = GP_ROTATE_90,
	/** @brief Rotate 180 degrees clockwise. */
	GP_ROTATE_180,
	/** @brief Rotate 270 degrees clockwise. */
	GP_ROTATE_270,
	/** @brief Rotate counter clockwise. Alias for GP_ROTATE_270 */
	GP_ROTATE_CCW = GP_ROTATE_270,
	/** @brief Mirror horizontally. */
	GP_MIRROR_H,
	/** @brief Mirror vertically. */
	GP_MIRROR_V,
	/** @brief Return value for invalid symmetry. */
	GP_ROTATE_INVALID = -1,
} gp_symmetry;

/**
 * @brief NULL-terminated array of symmetry names (C strings).
 */
extern const char **gp_symmetry_names;

/**
 * @brief Rotation by by name (as defined in gp_symmetry_names).
 *
 * @param symmetry A symmetry name.
 * @return One of the values defined in enum gp_symmetry.
 */
gp_symmetry gp_symmetry_by_name(const char *symmetry);

#endif /* CORE_GP_TYPES_H */
