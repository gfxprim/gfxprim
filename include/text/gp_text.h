/*****************************************************************************
 * This file is part of gfxprim library.                                     *
 *                                                                           *
 * Gfxprim is free software; you can redistribute it and/or                  *
 * modify it under the terms of the GNU Lesser General Public                *
 * License as published by the Free Software Foundation; either              *
 * version 2.1 of the License, or (at your option) any later version.        *
 *                                                                           *
 * Gfxprim is distributed in the hope that it will be useful,                *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of            *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU         *
 * Lesser General Public License for more details.                           *
 *                                                                           *
 * You should have received a copy of the GNU Lesser General Public          *
 * License along with gfxprim; if not, write to the Free Software            *
 * Foundation, Inc., 51 Franklin Street, Fifth Floor,                        *
 * Boston, MA  02110-1301  USA                                               *
 *                                                                           *
 * Copyright (C) 2009-2011 Jiri "BlueBear" Dluhos                            *
 *                         <jiri.bluebear.dluhos@gmail.com>                  *
 *                                                                           *
 * Copyright (C) 2009-2013 Cyril Hrubis <metan@ucw.cz>                       *
 *                                                                           *
 *****************************************************************************/

#ifndef TEXT_GP_TEXT_H
#define TEXT_GP_TEXT_H

#include <stdarg.h>

#include <core/gp_types.h>
#include <text/gp_text_style.h>
#include <text/gp_text_metric.h>
#include <text/gp_fonts.h>

/* How the rendered text should be aligned.
 * For gp_text(), the alignment is relative to the specified point:
 *
 *   - GP_ALIGN_LEFT draws the text to the left of the point,
 *   - GP_ALIGN_CENTER centers it at the point horizontally,
 *   - GP_ALIGN_RIGHT draws the text to the right of the point
 *   - GP_VALIGN_ABOVE (or TOP) draws the text above the point
 *   - GP_VALIGN_CENTER centers the text vertically at the point
 *   - GP_VALIGN_BASELINE places the text baseline at the point
 *   - GP_VALIGN_BELOW (or BOTTOM) draws the text below the point
 *   - GP_TEXT_NOBG mix the alpha pixels with data read from the pixmap
 *                  rather than mixing them with bg_color
 *                  (which is slightly slower)
 */
typedef enum gp_text_attr {
	GP_ALIGN_LEFT = 0x01,
	GP_ALIGN_CENTER = 0x02,
	GP_ALIGN_RIGHT = 0x03,
	GP_VALIGN_ABOVE = 0x10,
	GP_VALIGN_TOP = GP_VALIGN_ABOVE,
	GP_VALIGN_CENTER = 0x20,
	GP_VALIGN_BASELINE = 0x30,
	GP_VALIGN_BELOW = 0x40,
	GP_VALIGN_BOTTOM = GP_VALIGN_BELOW,
	GP_TEXT_NOBG = 0x80,
} gp_text_attr;

/*
 * Raw version, doesn't use Text aligment.
 *
 * If flags are set to GP_TEXT_NOBG the the bg_color is ignored and
 * the pixmap pixels are used for alpha mixing.
 */
void gp_text_raw(gp_pixmap *pixmap, const gp_text_style *style,
                 gp_coord x, gp_coord y, uint8_t flags,
                 gp_pixel fg_color, gp_pixel bg_color,
		 const char *str);

/*
 * Draws a string.
 *
 * The string is rendered to pixmap (horizontally) with defined text style.
 * The x and y coordinates determines point defined by aligment flags.
 *
 * The background color is ignored for 1bpp font formats.
 */
void gp_text(gp_pixmap *pixmap, const gp_text_style *style,
             gp_coord x, gp_coord y, int align,
             gp_pixel fg_color, gp_pixel bg_color, const char *str);

/*
 * Same as above, but printf like and returns text width in pixels.
 */
gp_size gp_print(gp_pixmap *pixmap, const gp_text_style *style,
                 gp_coord x, gp_coord y, int align,
	         gp_pixel fg_color, gp_pixel bg_color, const char *fmt, ...)
	         __attribute__ ((format (printf, 8, 9)));

gp_size gp_vprint(gp_pixmap *pixmap, const gp_text_style *style,
                  gp_coord x, gp_coord y, int align,
	          gp_pixel fg_color, gp_pixel bg_color,
                  const char *fmt, va_list va);
/*
 * Clears rectangle that would be used to draw text of size pixels.
 */
void gp_text_clear(gp_pixmap *pixmap, const gp_text_style *style,
                   gp_coord x, gp_coord y, int align,
		   gp_pixel bg_color, gp_size size);

/*
 * Dtto, but with string.
 */
void gp_text_clear_str(gp_pixmap *pixmap, const gp_text_style *style,
                       gp_coord x, gp_coord y, int align,
		       gp_pixel bg_color, const char *str);

#endif /* TEXT_GP_TEXT_H */
