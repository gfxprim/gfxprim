// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2009-2010 Jiri "BlueBear" Dluhos
 *                         <jiri.bluebear.dluhos@gmail.com>
 *
 * Copyright (C) 2009-2011 Cyril Hrubis <metan@ucw.cz>
 */

#ifndef TEXT_GP_TEXTSTYLE_H
#define TEXT_GP_TEXTSTYLE_H

#include <text/gp_font.h>

extern const gp_font_face gp_default_font;

/*
 * This structure describes how a text should be rendered.
 * It includes a font, and its various variants and transformations.
 */
typedef struct gp_text_style {
	const struct gp_font_face *font;

	/* Spacing between pixels (0 is the default, no spacing). */
	int pixel_xspace, pixel_yspace;

	/* Multiplier of pixel width/height (1 is default). */
	int pixel_xmul, pixel_ymul;

	/* Extra spacing (in pixels) between characters. */
	int char_xspace;
} gp_text_style;

#define GP_DEFAULT_TEXT_STYLE { \
	.font = &gp_default_font, \
	.pixel_xspace = 0, \
	.pixel_yspace = 0, \
	.pixel_xmul = 1, \
	.pixel_ymul = 1, \
	.char_xspace = 0 \
}

#endif /* TEXT_GP_TEXTSTYLE_H */
