// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2009-2017 Cyril Hrubis <metan@ucw.cz>
 */

#ifndef CORE_GP_TYPES_H
#define CORE_GP_TYPES_H

#include <stdint.h>

/* Integer type for coordinates i.e. x, y, ... */
typedef int gp_coord;

/* Integer type for sizes i.e. w, h, ... */
typedef unsigned int gp_size;

/* Pixel integer value packed accordingly to gp_pixel_type */
typedef uint32_t gp_pixel;

/* Pixel type description */
typedef struct gp_pixel_type_desc gp_pixel_type_desc;

/* Bitmap image */
typedef struct gp_pixmap gp_pixmap;

/* Gamma correction tables */
typedef struct gp_gamma gp_gamma;

/* Progress callback */
typedef struct gp_progress_cb gp_progress_cb;

#include <core/gp_seek.h>

#endif /* CORE_GP_TYPES_H */
