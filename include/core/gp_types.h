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

#endif /* CORE_GP_TYPES_H */
