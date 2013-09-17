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
 * Copyright (C) 2009-2013 Cyril Hrubis <metan@ucw.cz>                       *
 *                                                                           *
 *****************************************************************************/

#ifndef TEXT_GP_FONTS_H
#define TEXT_GP_FONTS_H

#include "GP_Font.h"

/*
 * Monospace 7x6 font
 *
 * Ascend: 5
 * Descent: 2
 * Width: 5 + 1
 */
extern const GP_FontFace *GP_FontTinyMono;

/*
 * Similar as FontTinyMono but proportional
 */
extern const GP_FontFace *GP_FontTiny;

/*
 * Monospace 8x8 Commodore 64 like font.
 */
extern const GP_FontFace *GP_FontC64;

#endif /* TEXT_GP_FONTS_H */
