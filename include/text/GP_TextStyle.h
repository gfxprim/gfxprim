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
 * Copyright (C) 2009-2011 Cyril Hrubis <metan@ucw.cz>                       *
 *                                                                           *
 *****************************************************************************/

#ifndef TEXT_GP_TEXTSTYLE_H
#define TEXT_GP_TEXTSTYLE_H

#include "GP_Font.h"
#include "GP_DefaultFont.h"

/*
 * This structure describes how a text should be rendered.
 * It includes a font, and its various variants and transformations.
 */
typedef struct GP_TextStyle {
	const struct GP_FontFace *font;

	/* Spacing between pixels (0 is the default, no spacing). */
	int pixel_xspace, pixel_yspace;

	/* Multiplier of pixel width/height (1 is default). */
	int pixel_xmul, pixel_ymul;

	/* Extra spacing (in pixels) between characters. */
	int char_xspace;

} GP_TextStyle;

#define GP_DEFAULT_TEXT_STYLE { \
	.font = &GP_DefaultConsoleFont, \
	.pixel_xspace = 0, \
	.pixel_yspace = 0, \
	.pixel_xmul = 1, \
	.pixel_ymul = 1, \
	.char_xspace = 0 \
}

#endif /* TEXT_GP_TEXTSTYLE_H */
