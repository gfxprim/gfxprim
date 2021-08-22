//SPDX-License-Identifier: LGPL-2.0-or-later
/*

   Copyright (c) 2014-2021 Cyril Hrubis <metan@ucw.cz>

 */

#ifndef GP_WIDGET_TATTR_H
#define GP_WIDGET_TATTR_H

#include <text/gp_text.h>
#include <widgets/gp_widget_types.h>
#include <widgets/gp_widget_render.h>

enum gp_widget_tattr {
	/* Fonts */
	GP_TATTR_BOLD  = 0x10,
	GP_TATTR_LARGE = 0x20,
	/* Monospace does not support large */
	GP_TATTR_MONO  = 0x40,
	GP_TATTR_FONT = 0x70,

	/* Horizontal alignment */
	GP_TATTR_LEFT = 0x01,
	GP_TATTR_CENTER = 0x02,
	GP_TATTR_RIGHT = 0x03,
	GP_TATTR_HALIGN = 0x03,
};

/*
 * @brief Returns text font based on text attributes and render context.
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

static inline int gp_widget_tattr_halign(gp_widget_tattr attr)
{
	return attr & GP_TATTR_HALIGN;
}

/*
 * @brief Parses an attributes encoded in a string into attribute bitflags.
 *
 * The attributes are separated by | e.g. valid attributes string is "bold|large".
 *
 * @attrs Text attributes description string.
 * @tattr A pointer to tattr to be filled in.
 * @flags A bitwise or of GP_TATTR_FONT and GP_TATTR_HALIGN that enables
 *        parsing font and/or alignment.
 * @return Zero on success, non-zero on parsing failure.
 */
int gp_widget_tattr_parse(const char *attrs, gp_widget_tattr *tattr, int flags);

#endif /* GP_WIDGET_TATTR_H */
