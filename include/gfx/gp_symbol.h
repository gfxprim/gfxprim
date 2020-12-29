// SPDX-License-Identifier: GPL-2.1-or-later
/*
 * Copyright (C) 2020 Cyril Hrubis <metan@ucw.cz>
 */

#ifndef GP_SYMBOL_H__
#define GP_SYMBOL_H__

#include "core/gp_types.h"

enum gp_symbol_type {
	GP_TRIANGLE_UP,
	GP_TRIANGLE_DOWN,
	GP_TRIANGLE_LEFT,
	GP_TRIANGLE_RIGHT,
};

void gp_symbol(gp_pixmap *pixmap, gp_coord x_center, gp_coord y_center,
               gp_size rx, gp_size ry, enum gp_symbol_type stype, gp_pixel pixel);

void gp_symbol_raw(gp_pixmap *pixmap, gp_coord x_center, gp_coord y_center,
                   gp_size rx, gp_size ry, enum gp_symbol_type stype, gp_pixel pixel);

#endif /* GP_SYMBOL_H__ */
