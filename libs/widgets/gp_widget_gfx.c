//SPDX-License-Identifier: LGPL-2.0-or-later

/*

   Copyright (c) 2014-2023 Cyril Hrubis <metan@ucw.cz>

 */

#include <string.h>
#include <widgets/gp_widget_gfx.h>
#include <widgets/gp_widget_render.h>
#include "gp_widgets_internal.h"

static gp_size text_size(gp_size add_size, const gp_text_style *style,
                         const char *str, size_t len)
{
	return add_size + gp_text_wbbox_len(style, str, len);
}

static size_t max_chars(const gp_text_style *style, gp_size w, const char *str)
{
	gp_size dots_size = gp_text_wbbox(style, "...");
	size_t left = 0, right = strlen(str);

	for (;;) {
		size_t mid = (left + right)/2;

		if (w > text_size(dots_size, style, str, mid))
			left = mid;
		else
			right = mid;

		if (left >= right - 1) {
			if (text_size(dots_size, style, str, right) <= w)
				return right;
			else
				return left;
		}
	}
}

void gp_text_fit(gp_pixmap *pix, const gp_text_style *style,
                gp_coord x, gp_coord y, gp_size w, int align,
		gp_pixel fg_color, gp_pixel bg_color, const char *str)
{
	gp_size text_w = gp_text_wbbox(style, str);

	if (text_w > w) {
		int chars = max_chars(style, w, str);

		align &= ~GP_ALIGN_HORIZ;
		align |= GP_ALIGN_RIGHT;

		gp_print(pix, style, x, y, align, fg_color, bg_color,
			 "%.*s...", chars, str);
		//gp_hline_xyw(pix, x, y, w, 0xff0000);
		return;
	}

	gp_text_xxy(pix, style, x, x+w-1, y, align, fg_color, bg_color, str);
}

void gp_rrect_xywh(gp_pixmap *pix, gp_coord x, gp_coord y,
                   gp_size w, gp_size h, gp_pixel color)
{
	unsigned int rs = ctx.fr_round;
	unsigned int th = ctx.fr_thick - 1;

	gp_coord lx = x + rs;
	gp_coord rx = x + w - rs - 1;
	gp_coord uy = y + rs;
	gp_coord dy = y + h - rs - 1;

	gp_fill_ring_seg(pix, lx, uy, rs-th, rs, GP_CIRCLE_SEG2, color);
	gp_fill_ring_seg(pix, rx, uy, rs-th, rs, GP_CIRCLE_SEG1, color);
	gp_fill_ring_seg(pix, lx, dy, rs-th, rs, GP_CIRCLE_SEG3, color);
	gp_fill_ring_seg(pix, rx, dy, rs-th, rs, GP_CIRCLE_SEG4, color);

	gp_fill_rect_xyxy(pix, lx, y, rx, y+th, color);
	gp_fill_rect_xyxy(pix, lx, y+h-1-th, rx, y+h-1, color);
	gp_fill_rect_xyxy(pix, x, uy, x+th, dy, color);
	gp_fill_rect_xyxy(pix, x+w-1, uy, x+w-1-th, dy, color);
}

void gp_fill_rrect_xywh(gp_pixmap *pix, gp_coord x, gp_coord y, gp_size w, gp_size h,
                        gp_pixel bg_color, gp_pixel fg_color, gp_pixel fr_color)
{
	unsigned int rs = ctx.fr_round;
	unsigned int th = ctx.fr_thick - 1;

	gp_coord lx = x + rs;
	gp_coord rx = x + w - rs - 1;
	gp_coord uy = y + rs;
	gp_coord dy = y + h - rs - 1;

	gp_fill_rect_xyxy(pix, x, y, lx, uy, bg_color);
	gp_fill_rect_xyxy(pix, rx, y, x+w-1, uy, bg_color);
	gp_fill_rect_xyxy(pix, x, dy, lx, y+h-1, bg_color);
	gp_fill_rect_xyxy(pix, rx, dy, x+w-1, y+h-1, bg_color);

	gp_fill_circle_seg(pix, lx, uy, rs, GP_CIRCLE_SEG2, fg_color);
	gp_fill_circle_seg(pix, rx, uy, rs, GP_CIRCLE_SEG1, fg_color);
	gp_fill_circle_seg(pix, lx, dy, rs, GP_CIRCLE_SEG3, fg_color);
	gp_fill_circle_seg(pix, rx, dy, rs, GP_CIRCLE_SEG4, fg_color);

	gp_fill_rect_xyxy(pix, lx, y+1, rx, y+h-2, fg_color);
	gp_fill_rect_xyxy(pix, x+1, uy, lx-1, dy, fg_color);
	gp_fill_rect_xyxy(pix, rx+1, uy, x+w-2, dy, fg_color);

	gp_fill_ring_seg(pix, lx, uy, rs-th, rs, GP_CIRCLE_SEG2, fr_color);
	gp_fill_ring_seg(pix, rx, uy, rs-th, rs, GP_CIRCLE_SEG1, fr_color);
	gp_fill_ring_seg(pix, lx, dy, rs-th, rs, GP_CIRCLE_SEG3, fr_color);
	gp_fill_ring_seg(pix, rx, dy, rs-th, rs, GP_CIRCLE_SEG4, fr_color);

	gp_fill_rect_xyxy(pix, lx, y, rx, y+th, fr_color);
	gp_fill_rect_xyxy(pix, lx, y+h-1-th, rx, y+h-1, fr_color);
	gp_fill_rect_xyxy(pix, x, uy, x+th, dy, fr_color);
	gp_fill_rect_xyxy(pix, x+w-1, uy, x+w-1-th, dy, fr_color);
}
