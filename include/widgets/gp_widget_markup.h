//SPDX-License-Identifier: LGPL-2.0-or-later
/*

   Copyright (c) 2014-2023 Cyril Hrubis <metan@ucw.cz>

 */

#ifndef GP_WIDGET_MARKUP_H
#define GP_WIDGET_MARKUP_H

#include <utils/gp_markup.h>
#include <utils/gp_markup_parser.h>
#include <utils/gp_markup_justify.h>

struct gp_widget_markup {
	unsigned int min_size_em;
	gp_markup_lines *lines;
	gp_markup *markup;
};

/**
 * @brief Allocates and initializes a markup widget.
 *
 * @markup A markup string.
 * @get A callback to resolve variables.
 *
 * @return A markup widget.
 */
gp_widget *gp_widget_markup_new(const char *markup, enum gp_markup_fmt fmt);

/**
 * @brief Sets new markup string.
 *
 * @self A markup widget.
 * @markup_str New markup string.
 *
 * @return Zero on success non-zero on a failure.
 */
int gp_widget_markup_set(gp_widget *self, enum gp_markup_fmt fmt,
		         const char *markup_str);

#endif /* GP_WIDGET_MARKUP_H */
