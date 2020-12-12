//SPDX-License-Identifier: LGPL-2.0-or-later

/*

   Copyright (c) 2014-2020 Cyril Hrubis <metan@ucw.cz>

 */

#ifndef GP_WIDGET_GFX_H__
#define GP_WIDGET_GFX_H__

#include <core/gp_core.h>
#include <gfx/gp_gfx.h>
#include <text/gp_text.h>

void gp_text_fit(gp_pixmap *pix, const gp_text_style *style,
                gp_coord x, gp_coord y, gp_size w, int align,
		gp_pixel fg_color, gp_pixel bg_color, const char *str);

void gp_fill_rrect_xywh(gp_pixmap *pix, gp_coord x, gp_coord y, gp_size w, gp_size h,
                        gp_pixel bg_color, gp_pixel fg_color, gp_pixel fr_color);

void gp_rrect_xywh(gp_pixmap *pix, gp_coord x, gp_coord y,
                   gp_size w, gp_size h, gp_pixel color);

void gp_triangle_up(gp_pixmap *pix, gp_coord x_center, gp_coord y_center,
                    gp_size base, gp_pixel color);

void gp_triangle_down(gp_pixmap *pix, gp_coord x_center, gp_coord y_center,
                      gp_size base, gp_pixel color);

void gp_triangle_updown(gp_pixmap *pix, gp_coord x_center, gp_coord y_center,
                        gp_size base, gp_pixel color);

#endif /* GP_WIDGET_GFX_H__ */
