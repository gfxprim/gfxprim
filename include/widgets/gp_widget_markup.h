//SPDX-License-Identifier: LGPL-2.0-or-later
/*

   Copyright (c) 2014-2024 Cyril Hrubis <metan@ucw.cz>

 */

/**
 * @file gp_widget_markup.h
 * @brief A text markup widget.
 *
 * Markup widget JSON attributes
 * ------------------------------
 *
 * | Attribute |  Type  |  Default  | Description                          |
 * |-----------|--------|-----------|--------------------------------------|
 * |  **fmt**  | string | "gfxprim" | A markup format enum #gp_markup_fmt. |
 * | **text**  | string |           | Markup text.                         |
 */
#ifndef GP_WIDGET_MARKUP_H
#define GP_WIDGET_MARKUP_H

#include <utils/gp_markup.h>
#include <utils/gp_markup_parser.h>
#include <utils/gp_markup_justify.h>

/**
 * @brief Allocates and initializes a markup widget.
 *
 * @param markup A markup string.
 * @param fmt A markup format.
 * @param flags Markup parser flags, depends on the markup format.
 *
 * @return A markup widget.
 */
gp_widget *gp_widget_markup_new(const char *markup, enum gp_markup_fmt fmt, int flags);

/**
 * @brief Sets new markup string.
 *
 * @param self A markup widget.
 * @param fmt A markup format.
 * @param flags Markup parser flags, depends on the markup format.
 * @param markup_str New markup string.
 *
 * @return Zero on success non-zero on a failure.
 */
int gp_widget_markup_set(gp_widget *self, enum gp_markup_fmt fmt,
		         int flags, const char *markup_str);

#endif /* GP_WIDGET_MARKUP_H */
