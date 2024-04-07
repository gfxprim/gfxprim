@ include header.t
/*
 * hline generated header
 *
 * Copyright (C) 2009-2014 Cyril Hrubis <metan@ucw.cz>
 */

@ for ps in pixelpacks:
void gp_hline_raw_{{ ps.suffix }}(gp_pixmap *pixmap, gp_coord x0,
			gp_coord x1, gp_coord y, gp_pixel pixel);

