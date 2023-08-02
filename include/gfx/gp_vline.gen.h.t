@ include header.t
/*
 * VLine generated header
 *
 * Copyright (C) 2009-2014 Cyril Hrubis <metan@ucw.cz>
 */

@ for ps in pixelpacks:
void gp_vline_raw_{{ ps.suffix }}(gp_pixmap *pixmap, gp_coord x,
			gp_coord y0, gp_coord y1, gp_pixel pixel);

void gp_vline_raw_{{ ps.suffix }}_clip(gp_pixmap *pixmap, gp_coord x,
			gp_coord y0, gp_coord y1, gp_pixel pixel);
