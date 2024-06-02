//SPDX-License-Identifier: LGPL-2.0-or-later
/*

   Copyright (c) 2014-2024 Cyril Hrubis <metan@ucw.cz>

 */

/**
 * @file gp_widget_tattr.h
 * @brief A text attrbutes.
 *
 * Text attributes describe text font, size, and horizontal alignment.
 * Attributes can be either passed down as a C constants, e.g. GP_TATTR_BOLD |
 * GP_TATTR_LARGE or parsed from a string from JSON e.g. "bold|large".
 *
 * @image html tattr_example.png
 */

#ifndef GP_WIDGET_TATTR_H
#define GP_WIDGET_TATTR_H

#include <text/gp_text.h>
#include <widgets/gp_widget_types.h>
#include <widgets/gp_widget_render.h>

/** @brief Text attribute flags. */
enum gp_widget_tattr {
	/** @brief Bold font. */
	GP_TATTR_BOLD  = 0x10,
	/** @brief Large font. */
	GP_TATTR_LARGE = 0x20,
	/** @brief Monospace font, can't be combined with large. */
	GP_TATTR_MONO  = 0x40,
	/** @brief Font attribute mask. */
	GP_TATTR_FONT = 0x70,

	/** @brief Left horizontal alignment. */
	GP_TATTR_LEFT = 0x01,
	/** @brief Center horizontal alignment. */
	GP_TATTR_CENTER = 0x02,
	/** @brief Right horizontal alignment. */
	GP_TATTR_RIGHT = 0x03,
	/** @brief Alignment attribute mask. */
	GP_TATTR_HALIGN = 0x03,
};

/**
 * @brief Returns text font based on text attributes and render context.
 *
 * @param attr Text attribute bitflags.
 * @param ctx A widget rendering context.
 * @return A text style to use for text rendering.
 */
static inline const gp_text_style *gp_widget_tattr_font(gp_widget_tattr attr, const gp_widget_render_ctx *ctx)
{
	if (attr & GP_TATTR_MONO) {
		if (attr & GP_TATTR_BOLD)
			return ctx->font_mono_bold;
		else
			return ctx->font_mono;
	}

	if (attr & GP_TATTR_BOLD) {
		if (attr & GP_TATTR_LARGE)
			return ctx->font_big_bold;
		else
			return ctx->font_bold;
	}

	if (attr & GP_TATTR_LARGE)
		return ctx->font_big;

	return ctx->font;
}

/**
 * @brief Returns the alignment part of the text attributes.
 *
 * @param attr A text attribute bitflags.
 * @return Horizontal aliment part of the flags.
 */
static inline int gp_widget_tattr_halign(gp_widget_tattr attr)
{
	return attr & GP_TATTR_HALIGN;
}

/**
 * @brief Parses text attributes encoded in a string into attribute bitflags.
 *
 * The attributes are separated by | e.g. valid attributes string is "bold|large".
 *
 * @param attrs Text attributes description string.
 * @param tattr A pointer to tattr to be filled in.
 * @param flags A bitwise or of GP_TATTR_FONT and GP_TATTR_HALIGN that enables
 *              parsing font and/or alignment.
 * @return Zero on success, non-zero on parsing failure.
 */
int gp_widget_tattr_parse(const char *attrs, gp_widget_tattr *tattr, int flags);

#endif /* GP_WIDGET_TATTR_H */
