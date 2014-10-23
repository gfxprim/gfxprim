@ include header.t
/*
 * VLine generated header
 *
 * Copyright (C) 2009-2014 Cyril Hrubis <metan@ucw.cz>
 */

@ for ps in pixelsizes:
void GP_VLine_Raw_{{ ps.suffix }}(GP_Context *context, GP_Coord x,
			GP_Coord y0, GP_Coord y1, GP_Pixel pixel);

