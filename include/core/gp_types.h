/*****************************************************************************
 * This file is part of gfxprim library.                                     *
 *                                                                           *
 * Gfxprim is free software; you can redistribute it and/or                  *
 * modify it under the terms of the GNU Lesser General Public                *
 * License as published by the Free Software Foundation; either              *
 * version 2.1 of the License, or (at your option) any later version.        *
 *                                                                           *
 * Gfxprim is distributed in the hope that it will be useful,                *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of            *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU         *
 * Lesser General Public License for more details.                           *
 *                                                                           *
 * You should have received a copy of the GNU Lesser General Public          *
 * License along with gfxprim; if not, write to the Free Software            *
 * Foundation, Inc., 51 Franklin Street, Fifth Floor,                        *
 * Boston, MA  02110-1301  USA                                               *
 *                                                                           *
 * Copyright (C) 2009-2017 Cyril Hrubis <metan@ucw.cz>                       *
 *                                                                           *
 *****************************************************************************/

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

#endif /* CORE_GP_TYPES_H */
