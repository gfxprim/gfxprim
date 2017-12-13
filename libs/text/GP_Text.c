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
 * Copyright (C) 2009-2014 Cyril Hrubis <metan@ucw.cz>                       *
 *                                                                           *
 *****************************************************************************/

#include <core/GP_FnPerBpp.h>
#include <core/GP_Debug.h>

#include <gfx/GP_Rect.h>

#include <text/GP_TextMetric.h>
#include <text/GP_Text.h>

gp_text_style gp_default_style = GP_DEFAULT_TEXT_STYLE;

static int do_align(gp_coord *topleft_x, gp_coord *topleft_y, int align,
                    gp_coord x, gp_coord y, const gp_text_style *style,
                    gp_size width)
{
	gp_size height = gp_text_height(style);

	switch (align & 0x0f) {
	case GP_ALIGN_LEFT:
		*topleft_x = x - width + 1;
		break;
	case GP_ALIGN_RIGHT:
		*topleft_x = x;
		break;
	case GP_ALIGN_CENTER:
		*topleft_x = x - width/2;
		break;
	default:
		GP_DEBUG(1, "ALIGN 0x%0x\n", align);
		return 1;
	}

	switch (align & 0x70) {
	case GP_VALIGN_ABOVE:
		*topleft_y = y - height + 1;
		break;
	case GP_VALIGN_CENTER:
		*topleft_y = y - height/2;
		break;
	case GP_VALIGN_BASELINE:
		*topleft_y = y - gp_text_ascent(style) + 1;
		break;
	case GP_VALIGN_BELOW:
		*topleft_y = y;
		break;
	default:
		GP_DEBUG(1, "VALIGN 0x%0x\n", align);
		return 1;
	}

	return 0;
}

void gp_text(gp_pixmap *pixmap, const gp_text_style *style,
             gp_coord x, gp_coord y, int align,
	     gp_pixel fg_color, gp_pixel bg_color,
             const char *str)
{
	GP_CHECK_PIXMAP(pixmap);

	if (str == NULL)
		return;

	if (style == NULL)
		style = &gp_default_style;

	gp_coord topleft_x, topleft_y;

	gp_size w = gp_text_width(style, str);

	GP_ASSERT(do_align(&topleft_x, &topleft_y, align, x, y, style, w) == 0,
	         "Invalid aligment flags");

	gp_text_raw(pixmap, style, topleft_x, topleft_y,
	            align & GP_TEXT_NOBG, fg_color, bg_color, str);
}

gp_size gp_vprint(gp_pixmap *pixmap, const gp_text_style *style,
                  gp_coord x, gp_coord y, int align,
                  gp_pixel fg_color, gp_pixel bg_color,
                  const char *fmt, va_list va)
{
	va_list vac;
	int size;

	va_copy(vac, va);
	size = vsnprintf(NULL, 0, fmt, va);
	char buf[size+1];
	vsnprintf(buf, sizeof(buf), fmt, vac);
	va_end(vac);

	gp_text(pixmap, style, x, y, align, fg_color, bg_color, buf);

	return gp_text_width(style, buf);
}

gp_size gp_print(gp_pixmap *pixmap, const gp_text_style *style,
                 gp_coord x, gp_coord y, int align,
                 gp_pixel fg_color, gp_pixel bg_color, const char *fmt, ...)
{
	va_list va;
	gp_size ret;

	va_start(va, fmt);
	ret = gp_vprint(pixmap, style, x, y, align,
	                fg_color, bg_color, fmt, va);
	va_end(va);

	return ret;
}

void gp_text_clear(gp_pixmap *pixmap, const gp_text_style *style,
                  gp_coord x, gp_coord y, int align,
		  gp_pixel bg_color, gp_size size)
{
	gp_coord topleft_x, topleft_y;

	GP_ASSERT(do_align(&topleft_x, &topleft_y, align, x, y, style, size) == 0,
	         "Invalid aligment flags");

	gp_fill_rect_xywh(pixmap, topleft_x, topleft_y,
	                  size, gp_text_height(style), bg_color);
}

void gp_text_clear_str(gp_pixmap *pixmap, const gp_text_style *style,
                       gp_coord x, gp_coord y, int align,
		       gp_pixel bg_color, const char *str)
{
	gp_text_clear(pixmap, style, x, y, align,
	              bg_color, gp_text_width(style, str));
}
