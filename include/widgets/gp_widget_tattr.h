//SPDX-License-Identifier: LGPL-2.0-or-later
/*

   Copyright (c) 2014-2021 Cyril Hrubis <metan@ucw.cz>

 */

#ifndef GP_WIDGET_TATTR_H__
#define GP_WIDGET_TATTR_H__

#include <text/gp_text.h>
#include <widgets/gp_widget_types.h>
#include <widgets/gp_widget_render.h>

enum gp_widget_tattr {
	GP_TATTR_BOLD  = 0x01,
	GP_TATTR_LARGE = 0x02,
};

/*
 * @brief Returns text font based on text attributes and render context.
 */
static inline const gp_text_style *gp_widget_tattr_font(gp_widget_tattr attr, const gp_widget_render_ctx *ctx)
{
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

/*
 * @brief Parses an attributes encoded in a string into attribute bitflags.
 *
 * The attributes are separated by | e.g. valid attributes string is "bold|large".
 *
 * @attrs Text attributes description string.
 * @tattr A pointe to tattr to be filled in.
 * @return Zero on success, non-zero on parsing failure.
 */
int gp_widget_tattr_parse(const char *attrs, gp_widget_tattr *tattr);

#endif /* GP_WIDGET_TATTR_H__ */
