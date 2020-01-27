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
 * Copyright (C) 2009-2010 Jiri "BlueBear" Dluhos                            *
 *                         <jiri.bluebear.dluhos@gmail.com>                  *
 *                                                                           *
 * Copyright (C) 2009-2014 Cyril Hrubis <metan@ucw.cz>                       *
 *                                                                           *
 *****************************************************************************/

/*

  This is header file for public core API.

 */

#ifndef GP_CORE_H
#define GP_CORE_H

/* Library verson */
#include "core/gp_version.h"

/* Common building blocks */
#include "core/gp_common.h"

/* Pixmap ... */
#include "core/gp_pixmap.h"

/* ... and it's trasformations */
#include <core/gp_transform.h>

/* Gamma */
#include <core/gp_gamma.h>

/* Pixeltypes */
#include <core/gp_pixel.h>

/* Pixel conversions */
#include <core/gp_convert.h>

/* Individual pixel access */
#include <core/gp_get_put_pixel.h>

/* Writing pixel blocks */
#include <core/gp_write_pixel.h>

/* Blitting */
#include <core/gp_blit.h>

/* Debug and debug level */
#include "core/gp_debug.h"

/* Progress callback */
#include <core/gp_progress_callback.h>

/* Threads utils */
#include <core/gp_threads.h>

/* Mix Pixel */
#include <core/gp_mix_pixels.h>

#include "core/gp_fill.h"

#endif /* GP_CORE_H */
