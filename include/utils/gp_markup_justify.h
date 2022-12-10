// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2022-2023 Cyril Hrubis <metan@ucw.cz>
 */

#ifndef UTILS_GP_MARKUP_JUSTIFY_H
#define UTILS_GP_MARKUP_JUSTIFY_H

#include <utils/gp_markup.h>

typedef struct gp_markup_line {
	gp_markup_glyph *first;
	gp_markup_glyph *last;
} gp_markup_line;

typedef struct gp_markup_lines {
	unsigned int lines_cnt;
	gp_markup_line lines[];
} gp_markup_lines;

typedef unsigned int (*gp_markup_width_cb)(gp_markup_glyph *first, size_t len, void *priv);

gp_markup_lines *gp_markup_justify(gp_markup *self, unsigned int line_width, gp_markup_width_cb width_callback, void *priv);

void gp_markup_justify_free(gp_markup_lines *self);

void gp_markup_justify_dump(gp_markup_lines *self);

#endif /* UTILS_GP_MARKUP_JUSTIFY_H */
