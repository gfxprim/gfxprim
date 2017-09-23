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
 * Copyright (C) 2009-2012 Cyril Hrubis <metan@ucw.cz>                       *
 *                                                                           *
 *****************************************************************************/

 /*

   Backend test.

   1. Draw to the backend
   2. Sleep
   3. Exit

  */

#include <math.h>

#include <GP.h>
#include <backends/GP_Backends.h>

static void draw(GP_Pixmap *pixmap, int x, int y, int l)
{
	int x2, y2, x3, y3;

	GP_Pixel red, blue, green;
	
	red = GP_RGBToPixmapPixel(255,   0,   0, pixmap);
	blue = GP_RGBToPixmapPixel(0,   0, 255, pixmap);
	green = GP_RGBToPixmapPixel(0, 255,   0, pixmap);

	x2 = x + l/2;
	y2 = y + sqrt(2)/2 * l;
	x3 = x - l/2;
	y3 = y2;

	GP_FillTriangle(pixmap,  x,  y, x2, y2, x + l, y, red);
	GP_FillTriangle(pixmap,  x,  y, x3, y3, x - l, y, green);
	GP_FillTriangle(pixmap, x2, y2, x3, y3, x, y + sqrt(2) * l, blue);
}

int main(void)
{
	GP_Backend *backend;
	GP_Pixmap *pixmap;
	GP_TextStyle style;

	GP_SetDebugLevel(10);

	backend = GP_BackendLinuxFBInit("/dev/fb0", 0);
	
	if (backend == NULL) {
		fprintf(stderr, "Failed to initalize framebuffer\n");
		return 1;
	}
	
	pixmap = backend->pixmap;
	
	GP_DefaultTextStyle(&style);

	style.pixel_xspace = 2;
	style.pixel_ymul   = 2;

	GP_Pixel gray, black;

	gray = GP_RGBToPixmapPixel(200, 200, 200, pixmap);
	black = GP_RGBToPixmapPixel(0, 0, 0, pixmap);
	
	const char *text = "Framebuffer test";

	GP_Fill(pixmap, gray);
	GP_Line(pixmap, 0, 0, pixmap->w, pixmap->h, black);
	GP_Line(pixmap, 0, pixmap->h, pixmap->w, 0, black);
	GP_Text(pixmap, &style,
	        (pixmap->w - GP_TextWidth(&style, text))/2,
		16, GP_ALIGN_RIGHT|GP_VALIGN_BELOW, black, gray, text);


	draw(pixmap, pixmap->w / 2, 2.00 * pixmap->h / 3, 60);

	GP_BackendFlip(backend);

	sleep(10);

	GP_BackendExit(backend);

	return 0;
}
