@ include header.t
/*
 * HLine generated header
 *
 * Copyright (C) 2009-2014 Cyril Hrubis <metan@ucw.cz>
 */

@ for ps in pixelsizes:
void GP_HLine_Raw_{{ ps.suffix }}(GP_Context *context, GP_Coord x0,
			GP_Coord x1, GP_Coord y, GP_Pixel pixel);

