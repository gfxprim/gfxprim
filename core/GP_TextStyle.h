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

#ifndef GP_TEXTSTYLE_H
#define GP_TEXTSTYLE_H

#include "GP_Font.h"

#include <stdint.h>

/*
 * This structure describes how a text should be rendered.
 * It includes a font, and its various variants and transformations.
 */
struct GP_TextStyle {

	/* Font to use, or NULL to use the default font. */
	struct GP_Font *font;

	/* Spacing between pixels (0 is the default, no spacing). */
	int pixel_xspace, pixel_yspace;

	/* Multiplier of pixel width/height (1 is default). */
	int pixel_xmul, pixel_ymul;
};

/*
 * Static initializer for initializing a GP_TextStyle structure to default
 * values.
 * Note that at least the colors should always be changed afterwards,
 * as there is no sensible default (they are initialized to 0).
 */
#define GP_DEFAULT_TEXT_STYLE { NULL, 0, 0, 1, 1 }

/*
 * Initalize text style to the default values.
 */
void GP_DefaultTextStyle(struct GP_TextStyle *style); 

#endif /* GP_TEXTSTYLE_H */
