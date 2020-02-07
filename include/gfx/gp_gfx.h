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
 * Copyright (C) 2009-2013 Cyril Hrubis <metan@ucw.cz>                       *
 *                                                                           *
 *****************************************************************************/

/*

  This is a main header for gfx part.

 */

#ifndef GP_GFX_H
#define GP_GFX_H

/* basic definitions and structures */
#include "core/gp_pixmap.h"
#include <core/gp_get_put_pixel.h>
#include <core/gp_write_pixel.h>
#include "core/gp_fill.h"

/* public drawing API */
#include <gfx/gp_hline.h>
#include <gfx/gp_vline.h>
#include <gfx/gp_line.h>
#include <gfx/gp_rect.h>
#include <gfx/gp_triangle.h>
#include <gfx/gp_tetragon.h>
#include <gfx/gp_circle.h>
#include <gfx/gp_circle_seg.h>
#include <gfx/gp_ellipse.h>
#include <gfx/gp_arc.h>
#include <gfx/gp_polygon.h>

#include <gfx/gp_putpixel_aa.h>
#include <gfx/gp_vline_aa.h>
#include <gfx/gp_hline_aa.h>
#include <gfx/gp_line_aa.h>

#endif /* GP_GFX_H */
