// SPDX-License-Identifier: GPL-2.1-or-later
/*
 * Copyright (C) 2009-2011 Jiri "BlueBear" Dluhos
 *                         <jiri.bluebear.dluhos@gmail.com>
 *
 * Copyright (C) 2009-2014 Cyril Hrubis <metan@ucw.cz>
 */

#include <core/gp_pixmap.h>
#include <core/gp_fn_per_bpp.h>
#include <core/gp_debug.h>

#include <gfx/gp_rect.h>

#include <text/gp_text_metric.h>
#include <text/gp_text.h>

gp_text_style gp_default_style = GP_DEFAULT_TEXT_STYLE;

static int do_align(gp_coord *topleft_x, gp_coord *topleft_y, int align,
                    gp_coord x, gp_coord y, const gp_text_style *style,
                    gp_size width)
{
	gp_size height = gp_text_height(style);

	switch (align & 0x03) {
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

gp_size gp_text(gp_pixmap *pixmap, const gp_text_style *style,
               gp_coord x, gp_coord y, int align,
	       gp_pixel fg_color, gp_pixel bg_color,
               const char *str)
{
	return gp_text_ext(pixmap, style, x, y, align, fg_color, bg_color, str, SIZE_MAX);
}

gp_size gp_text_ext(gp_pixmap *pixmap, const gp_text_style *style,
                    gp_coord x, gp_coord y, int align,
	            gp_pixel fg_color, gp_pixel bg_color,
                    const char *str, size_t max_chars)
{
	GP_CHECK_PIXMAP(pixmap);

	if (str == NULL)
		return 0;

	if (style == NULL)
		style = &gp_default_style;

	gp_coord topleft_x, topleft_y;

	gp_size w = gp_text_width(style, str);

	GP_ASSERT(do_align(&topleft_x, &topleft_y, align, x, y, style, w) == 0,
	         "Invalid aligment flags");

	return gp_text_raw(pixmap, style, topleft_x, topleft_y,
	                   align, fg_color, bg_color, str, max_chars);
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

	return gp_text(pixmap, style, x, y, align, fg_color, bg_color, buf);
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
