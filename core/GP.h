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
 * Copyright (C) 2009-2010 Cyril Hrubis <metan@ucw.cz>                       *
 *                                                                           *
 *****************************************************************************/

#ifndef GP_H
#define GP_H

#include <stdint.h>

/* basic definitions and structures */
#include "GP_abort.h"
#include "GP_check.h"
#include "GP_minmax.h"
#include "GP_swap.h"
#include "GP_context.h"

/* semi-public, low-level drawing API */
#include "GP_readpixel.h"
#include "GP_writepixel.h"
#include "GP_fillcolumn.h"
#include "GP_fillrow.h"

/* colors */
#include "GP_color.h"
#include "GP_palette.h"

/* public drawing API */
#include "GP_getpixel.h"
#include "GP_putpixel.h"
#include "GP_hline.h"
#include "GP_vline.h"
#include "GP_line.h"
#include "GP_rect.h"
#include "GP_fillrect.h"
#include "GP_triangle.h"
#include "GP_filltriangle.h"
#include "GP_circle.h"
#include "GP_fillcircle.h"
#include "GP_ellipse.h"
#include "GP_fillellipse.h"

/* fonts */
#include "GP_font.h"
#include "GP_textstyle.h"
#include "GP_textmetric.h"
#include "GP_text.h"

#endif /* GP_H */
