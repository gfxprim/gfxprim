//SPDX-License-Identifier: LGPL-2.0-or-later

/*

   Copyright (c) 2014-2020 Cyril Hrubis <metan@ucw.cz>

 */

#include <string.h>
#include <widgets/gp_widget_gfx.h>

static gp_size text_size(gp_size add_size, const gp_text_style *style,
                         const char *str, size_t len)
{
	return add_size + gp_text_width_len(style, str, len);
}

static size_t max_chars(const gp_text_style *style, gp_size w, const char *str)
{
	gp_size dots_size = gp_text_width(style, "...");
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
	gp_size text_w = gp_text_width(style, str);

	if (text_w > w) {
		int chars = max_chars(style, w, str);

		gp_print(pix, style, x, y, align, fg_color, bg_color,
			 "%.*s...", chars, str);
		//gp_hline_xyw(pix, x, y, w, 0xff0000);
		return;
	}

	gp_text(pix, style, x, y, align, fg_color, bg_color, str);
}

void gp_rrect_xywh(gp_pixmap *pix, gp_coord x, gp_coord y,
                   gp_size w, gp_size h, gp_pixel color)
{
	unsigned int rs = 3;

	gp_coord lx = x + rs;
	gp_coord rx = x + w - rs - 1;
	gp_coord uy = y + rs;
	gp_coord dy = y + h - rs - 1;

	gp_circle_seg(pix, lx, uy, rs, GP_CIRCLE_SEG2, color);
	gp_circle_seg(pix, rx, uy, rs, GP_CIRCLE_SEG1, color);
	gp_circle_seg(pix, lx, dy, rs, GP_CIRCLE_SEG3, color);
	gp_circle_seg(pix, rx, dy, rs, GP_CIRCLE_SEG4, color);

	gp_hline_xxy(pix, lx, rx, y, color);
	gp_hline_xxy(pix, lx, rx, y+h-1, color);
	gp_vline_xyy(pix, x, uy, dy, color);
	gp_vline_xyy(pix, x+w-1, uy, dy, color);
}

void gp_fill_rrect_xywh(gp_pixmap *pix, gp_coord x, gp_coord y, gp_size w, gp_size h,
                        gp_pixel bg_color, gp_pixel fg_color, gp_pixel fr_color)
{
	unsigned int rs = 3;

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

	gp_circle_seg(pix, lx, uy, rs, GP_CIRCLE_SEG2, fr_color);
	gp_circle_seg(pix, rx, uy, rs, GP_CIRCLE_SEG1, fr_color);
	gp_circle_seg(pix, lx, dy, rs, GP_CIRCLE_SEG3, fr_color);
	gp_circle_seg(pix, rx, dy, rs, GP_CIRCLE_SEG4, fr_color);

	gp_hline_xxy(pix, lx, rx, y, fr_color);
	gp_hline_xxy(pix, lx, rx, y+h-1, fr_color);
	gp_vline_xyy(pix, x, uy, dy, fr_color);
	gp_vline_xyy(pix, x+w-1, uy, dy, fr_color);
}
