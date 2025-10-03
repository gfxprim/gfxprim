@ include source.t
/*
 * Vertical Line drawing algorithm.
 *
 * Copyright (C) 2009-2014 Cyril Hrubis <metan@ucw.cz>
 */

#include <core/gp_get_put_pixel.h>
#include <gfx/gp_vline.gen.h>

@ for ps in pixelpacks:
void gp_vline_raw_{{ ps.suffix }}(gp_pixmap *pixmap, gp_coord x,
			gp_coord y0, gp_coord y1, gp_pixel pixel)
{
	int y;
@     if ps.size != 1:
	for (y = y0; y <= y1; y++)
		gp_putpixel_raw_{{ ps.suffix }}(pixmap, x, y, pixel);
@     else:
	gp_pixel fg, bg, p;
	uint8_t x_mod, y_mod, *lookup;

	switch (gp_pixel_pattern_get(pixel)) {
	case GP_PIXEL_PATTERN_NONE:
		for (y = y0; y <= y1; y++)
			gp_putpixel_raw_{{ ps.suffix }}(pixmap, x, y, pixel);
	break;
	default:
		lookup = gp_pixel_pattern_lookup_table_get(gp_pixel_pattern_get(pixel));
		fg = gp_pixel_pattern_fg_get(pixel);
		bg = gp_pixel_pattern_bg_get(pixel);
		x_mod = (pixmap->offset + x) % GP_PIXEL_PATTERN_W;

		for (y = y0; y <= y1; y++) {
			y_mod = y % GP_PIXEL_PATTERN_H;
			gp_pixel p = gp_pixel_pattern_pixel_get(lookup, fg, bg, x_mod, y_mod);
			gp_putpixel_raw_{{ ps.suffix }}(pixmap, x, y, p);
		}
	break;
	}
@     end
}

void gp_vline_raw_{{ ps.suffix }}_clip(gp_pixmap *pixmap, gp_coord x,
                                       gp_coord y0, gp_coord y1, gp_pixel pixel)
{
	if (x < 0 || x >= (gp_coord) pixmap->w ||
	    y1 < 0 || y0 >= (gp_coord) pixmap->h)
		return;

	y0 = GP_MAX(y0, 0);
	y1 = GP_MIN(y1, (gp_coord) pixmap->h - 1);

	gp_vline_raw_{{ ps.suffix }}(pixmap, x, y0, y1, pixel);
}
