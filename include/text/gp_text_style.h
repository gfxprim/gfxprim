// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2009-2010 Jiri "BlueBear" Dluhos
 *                         <jiri.bluebear.dluhos@gmail.com>
 *
 * Copyright (C) 2009-2011 Cyril Hrubis <metan@ucw.cz>
 */

/**
 * @file gp_text_style.h
 * @brief A text style.
 */

#ifndef TEXT_GP_TEXTSTYLE_H
#define TEXT_GP_TEXTSTYLE_H

#include <text/gp_font.h>

extern const gp_font_face gp_default_font;

/**
 * @brief A text style.
 *
 * This structure describes how a text should be rendered.  It includes a font,
 * and a few transformations.
 */
typedef struct gp_text_style {
	/** @brief A font face. */
	const gp_font_face *font;

	/**
	 * @brief Spacing between glyph pixels.
	 *
	 * Default spacing is 0 i.e. no spacing.
	 */
	int pixel_xspace;
	/**
	 * @brief Spacing between glyph pixels.
	 *
	 * Default spacing is 0 i.e. no spacing.
	 */
	int pixel_yspace;

	/**
	 * @brief An x multiplier of glyps pixels i.e. width (1 is default).
	 */
	int pixel_xmul;
	/**
	 * @brief An y multiplier of glyps pixels height (1 is default).
	 */
	int pixel_ymul;

	/**
	 * @brief Extra spacing (in pixels) between characters.
	 */
	int char_xspace;
} gp_text_style;

/**
 * @brief A default text style initializer.
 */
#define GP_DEFAULT_TEXT_STYLE { \
	.font = &gp_default_font, \
	.pixel_xspace = 0, \
	.pixel_yspace = 0, \
	.pixel_xmul = 1, \
	.pixel_ymul = 1, \
	.char_xspace = 0 \
}

/**
 * @brief Sets normal text style flags.
 *
 * @param style A text style to intialize.
 * @param font A font face to use in the style.
 * @param mul A text style pixel multiplier.
 */
static inline void gp_text_style_normal(gp_text_style *style, const gp_font_face *font, int mul)
{
	style->pixel_xspace = 0;
	style->pixel_yspace = 0;
	style->pixel_xmul = mul;
	style->pixel_ymul = mul;
	style->font = font;
}

/**
 * @brief Sets style flags to embold a font.
 *
 * Creates a bold variant of a bitmap font on the expense of slower rendering.
 *
 * @param style A text style to intialize.
 * @param font A font face to use in the style.
 * @param mul A text style pixel multiplier.
 */
static inline void gp_text_style_embold(gp_text_style *style, const gp_font_face *font, int mul)
{
	style->pixel_xspace = -1;
	style->pixel_yspace = -1;
	style->pixel_xmul = mul + 1;
	style->pixel_ymul = mul + 1;
	style->font = font;
}

#endif /* TEXT_GP_TEXTSTYLE_H */
